#include "decodethread.h"

const QString decodeWasSuccess("decoding seems okay");

DecodeThread::DecodeThread(const QString &decodeFile, int codec, QObject *parent) : QThread(parent),
    m_abort(false), decodeFile(decodeFile), codec(codec)
{
    qDebug() << "Decoder construct Thread : " << QThread::currentThreadId() << "," << "decodeFileName:" << decodeFile << "," << "codec:" << codec;
}

DecodeThread::~DecodeThread()
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();

    wait();
}

void DecodeThread::run()
{
    qDebug() << "decoder Thread : " << QThread::currentThreadId();

    QTemporaryFile *decodeResult = new QTemporaryFile("decoder"); //todo free memory
    if (decodeResult->open()){
        qDebug() << "decodeResult:" << decodeResult->fileName();
    }
    if (codec == 0) {// or codec == 1) {
        FILE * file_speech, *file_analysis;
        short synth[160];
        int frames = 0;
        int * destate;
        int read_size;
        unsigned char analysis[32];
        enum Mode dec_mode;
        char magic[8];
        short block_size[16]={ 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };

        file_speech = fopen(qPrintable(decodeResult->fileName()), "wb");
        if (file_speech == NULL){
           //fprintf ( stderr, "%s%s%s\n","Use: ",argv[0], " input.file output.file " );
           qDebug() << 1;
        }

        file_analysis = fopen(qPrintable(decodeFile), "rb");
        if (file_analysis == NULL){
           //fprintf ( stderr, "%s%s%s\n","Use: ",argv[0], " input.file output.file " );
           fclose(file_speech);
           qDebug() << 1;
        }

        destate = (int *)Decoder_Interface_init();
        while (fread(analysis, sizeof (unsigned char), 1, file_analysis ) > 0)
        {
           dec_mode = (Mode)((analysis[0] >> 3) & 0x000F);
           read_size = block_size[dec_mode];
           fread(&analysis[1], sizeof (char), read_size, file_analysis );
           frames ++;

           Decoder_Interface_Decode(destate, analysis, synth, 0);

           fwrite( synth, sizeof (short), 160, file_speech );
        }

        Decoder_Interface_exit(destate);

        fclose(file_speech);
        fclose(file_analysis);
    }
    else if (codec == 1) {// or codec == 4) {
        FILE *f_serial;                        /* File of serial bits for transmission  */
        FILE *f_synth;                         /* File of speech data                   */
        Word16 synth[L_FRAME16k];              /* Buffer for speech @ 16kHz             */
        UWord8 serial[NB_SERIAL_MAX], pktBuffer[NB_SERIAL_MAX];
        Word16 mode;
        Word32 frame;
        char magic[16];
        void *st;
        unsigned char FTList[12], FTCount;    //max number of amr frame in one packet
        unsigned short pktLen;
        unsigned char currentFTptr = 4;
        unsigned int bytePtr = 0;
        if ((f_serial = fopen(qPrintable(decodeFile), "rb")) == NULL)
        {
            qDebug() << "error open input file:" << decodeFile;
            return;
        }

        st = D_IF_init();
        frame = 0;
        while(fread(&pktLen, sizeof(unsigned short), 1, f_serial)) {
            //qDebug() << pktLen;
            FTCount = 0;
            bytePtr = 0;
            currentFTptr = 4;
            if (fread(pktBuffer, sizeof (UWord8), pktLen, f_serial )==0){     //read whole packet
                qDebug() << "error reading packets";
                break;
            }
            while (FTCount < 12 and bytePtr < pktLen) {
                if(currentFTptr > 3 and currentFTptr < 7) {
                    FTList[FTCount++] =  (pktBuffer[bytePtr] << (currentFTptr - 3)) & 0x0f | ( pktBuffer[bytePtr + 1] >> (11 - currentFTptr));
                }
                else if (currentFTptr <= 3) {
                    FTList[FTCount++] = ( pktBuffer[bytePtr] >> (4 - currentFTptr)) & 0x0f;
                }
                else if (currentFTptr == 7){
                    FTList[FTCount++] = ( pktBuffer[bytePtr + 1] >> 4) & 0x0f;
                }

                //qDebug() << pktBuffer[bytePtr] << " " << (0x1 << (7-currentFTptr));
                if ((pktBuffer[bytePtr] & (0x1 << (7-currentFTptr))) == 0)
                    break;
                else {
                    currentFTptr += 6;
                    if (currentFTptr > 7) {

                    }
                    else {
                        if (currentFTptr == 7){
                            FTList[FTCount++] = ( pktBuffer[bytePtr + 1] >> 4) & 0x0f;
                        }
                        else {
                            FTList[FTCount++] =  (pktBuffer[bytePtr] << (currentFTptr - 3)) & 0x0f | ( pktBuffer[bytePtr + 1] >> (11 - currentFTptr));
                        }
                        currentFTptr += 6;
                    }
                    currentFTptr %= 8;
                    ++bytePtr;
                }
            }
            currentFTptr += 6;
            bytePtr = bytePtr + currentFTptr / 8;
            currentFTptr %= 8;
            for (int i=0; i<FTCount; ++i) {
                //qDebug() << FTList[i] << ":" << amr_wb_nob[FTList[i]];
                serial[0] = FTList[i] << 3 | 0x4;
                int serialByteLen = qCeil(amr_wb_nob[FTList[i]] / 8.0);
                int serialByteMod = amr_wb_nob[FTList[i]] % 8;
                for( int j = 1; j <= serialByteLen and bytePtr < pktLen; ++j, ++bytePtr) {
                    if (j < serialByteLen) {
                        if (currentFTptr == 7) {
                            serial[j] = pktBuffer[bytePtr + 1];
                        }
                        else {
                            serial[j] = pktBuffer[bytePtr]<<currentFTptr | pktBuffer[bytePtr + 1]>>(8 - currentFTptr);
                        }
                    }
                    else {
                        if (currentFTptr == 7) {
                            serial[j] = pktBuffer[bytePtr + 1] & 0xff << (8 - serialByteMod);
                        }
                        else {
                            serial[j] = pktBuffer[bytePtr]<<currentFTptr & 0xff<<(8 - serialByteMod);
                        }
                    }
                }
                currentFTptr = (currentFTptr + serialByteMod) % 8;
            }
            frame++;
            D_IF_decode( st, serial, synth, _good_frame);
            decodeResult->write((char *)synth, L_FRAME16k * 2);
        }


//        while (fread(serial, sizeof (UWord8), 2, f_serial ) > 0)
//        {
//            FTCount = 0;
//            //while ()
//            mode = (Word16)((serial[1] >> 7) & 0x0F);
//            qDebug() << mode << block_size[mode] - 1;
//            fread(&serial[1], sizeof (UWord8), block_size[mode] - 1, f_serial );
//            frame++;
//            fprintf(stderr, " Decoding frame: %ld\r", frame);
//            D_IF_decode( st, serial, synth, _good_frame);
//            fwrite(synth, sizeof(Word16), L_FRAME16k, f_synth);
//            fflush(f_synth);
//        }
        D_IF_exit(st);
        fclose(f_serial);
        decodeResult->close();
        qDebug() << decodeResult->fileName();
        //fclose(f_synth);
        qDebug() << frame;
    }
/*
    while((res = pcap_next_ex( fp, &header, &pkt_data)) >= 0)
    {
        if (m_abort)    return;

        if (100 * ++currentPacketNo / packetCount > currentProgress) {
            emit updateProgress(++currentProgress);
        }
*/

    emit decodeSuccess(decodeResult);
    emit lastWords(decodeWasSuccess);
    //pcap_close (fp);
}

void DecodeThread::stopMe()
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();
}
