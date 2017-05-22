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
        FILE *f_serial;                        /* File of serial bits for transmission  */
        Word16 synth[160];              /* Buffer for speech @ 8kHz             */
        UWord8 serial[32], pktBuffer[32];
        Word32 frame;
        void *st;
        unsigned char FTList[12], FTCount;    //max number of amr frame in one packet 12
        unsigned short pktLen;                //current packet length
        unsigned char currentFTptr = 4;       //FT pointer and amr data pointer
        unsigned int bytePtr = 0;             //current byte pointer
        if ((f_serial = fopen(qPrintable(decodeFile), "rb")) == NULL)
        {
            qDebug() << "error open input file:" << decodeFile;
            return;
        }

        st = Decoder_Interface_init();

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
            /* get all FT into an array, max len 12 */
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

            /* for each amr data in each pkt, decode follow FT indicate */
            for (int i=0; i<FTCount; ++i) {
                serial[0] = FTList[i] << 3 | 0x4;
                int serialByteLen = qCeil(amr_nb_nob[FTList[i]] / 8.0);    //useful amr data len in bytes
                int serialByteMod = amr_nb_nob[FTList[i]] % 8;             //last amr data byte, bit len
                for( int j = 1; j <= serialByteLen and bytePtr < pktLen; ++j, ++bytePtr) {
                    if (j < serialByteLen) {
                        if (currentFTptr == 7) {
                            serial[j] = pktBuffer[bytePtr + 1];
                        }
                        else {
                            serial[j] = pktBuffer[bytePtr]<<currentFTptr | pktBuffer[bytePtr + 1]>>(8 - currentFTptr);
                        }
                    }
                    else {    //last byte
                        if (currentFTptr == 7) {
                            serial[j] = pktBuffer[bytePtr + 1];// & 0xff << (8 - serialByteMod);
                        }
                        else {
                            serial[j] = pktBuffer[bytePtr]<<currentFTptr;// & 0xff<<(8 - serialByteMod);
                        }
                    }
                }
                currentFTptr = (currentFTptr + serialByteMod) % 8;
            }
            frame++;

            Decoder_Interface_Decode(st, serial, synth, 0);
            decodeResult->write((char *)synth, 160 * 2);
        }

        Decoder_Interface_exit(st);

        fclose(f_serial);
        decodeResult->close();
        qDebug() << decodeResult->fileName();
        qDebug() << frame;

    }
    else if (codec == 1) {// or codec == 4) {
        FILE *f_serial;                        /* File of serial bits for transmission  */
        Word16 synth[L_FRAME16k];              /* Buffer for speech @ 16kHz             */
        UWord8 serial[NB_SERIAL_MAX], pktBuffer[NB_SERIAL_MAX];
        Word32 frame;
        void *st;
        unsigned char FTList[12], FTCount;    //max number of amr frame in one packet 12
        unsigned short pktLen;                //current packet length
        unsigned char currentFTptr = 4;       //FT pointer and amr data pointer
        unsigned int bytePtr = 0;             //current byte pointer
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
            /* get all FT into an array, max len 12 */
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

            /* for each amr data in each pkt, decode follow FT indicate */
            for (int i=0; i<FTCount; ++i) {
                serial[0] = FTList[i] << 3 | 0x4;
                int serialByteLen = qCeil(amr_wb_nob[FTList[i]] / 8.0);    //useful amr data len in bytes
                int serialByteMod = amr_wb_nob[FTList[i]] % 8;             //last amr data byte, bit len
                for( int j = 1; j <= serialByteLen and bytePtr < pktLen; ++j, ++bytePtr) {
                    if (j < serialByteLen) {
                        if (currentFTptr == 7) {
                            serial[j] = pktBuffer[bytePtr + 1];
                        }
                        else {
                            serial[j] = pktBuffer[bytePtr]<<currentFTptr | pktBuffer[bytePtr + 1]>>(8 - currentFTptr);
                        }
                    }
                    else {    //last byte
                        if (currentFTptr == 7) {
                            serial[j] = pktBuffer[bytePtr + 1];// & 0xff << (8 - serialByteMod);
                        }
                        else {
                            serial[j] = pktBuffer[bytePtr]<<currentFTptr;// & 0xff<<(8 - serialByteMod);
                        }
                    }
                }
                currentFTptr = (currentFTptr + serialByteMod) % 8;
            }
            frame++;
            D_IF_decode( st, serial, synth, _good_frame);
            decodeResult->write((char *)synth, L_FRAME16k * 2);
        }

        D_IF_exit(st);
        fclose(f_serial);
        decodeResult->close();
        qDebug() << decodeResult->fileName();
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
