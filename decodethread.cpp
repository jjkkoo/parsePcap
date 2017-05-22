#include "decodethread.h"

using std::bitset;

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
    else if (codec == 1){// or codec == 4) {
        FILE *f_serial;                        /* File of serial bits for transmission  */
        FILE *f_synth;                         /* File of speech data                   */
        Word16 synth[L_FRAME16k];              /* Buffer for speech @ 16kHz             */
        UWord8 serial[NB_SERIAL_MAX];
        Word16 mode;
        Word32 frame;
        char magic[16];
        void *st;
        unsigned char FTList[12], FTCount;    //max number of amr frame in one packet
        unsigned short mediaLen;
        int currentFT = 4;
        if ((f_serial = fopen(qPrintable(decodeFile), "rb")) == NULL)
        {
            qDebug() << "error open input file:" << decodeFile;
            return;
        }

        st = D_IF_init();
        frame = 0;
        while(fread(mediaLen, sizeof(unsigned short), 1, f_serial)) {
            qDebug() << mediaLen;
            FTCount = 0;
            while(fread(serial, sizeof (UWord8), mediaLen, f_serial )) {    //read whole packet
                int currentByte
                for(int i = 0; i < 9; ++i){    //enough for search FT
                    bitset<8> bs(*serial);
                    if (bs[currentFT]==0 or i >mediaLen)
                        break;
                    else {

                    }
                }

                FTList[FTCount] = tempChar
                if (FTList[FTCount] & fBitMark != 0) {
                    fBitMark = fBitMark >> 6;
                }
                else {
                    break;
                }
            }
        }


        while (fread(serial, sizeof (UWord8), 2, f_serial ) > 0)
        {
            FTCount = 0;
            //while ()
            mode = (Word16)((serial[1] >> 7) & 0x0F);
            qDebug() << mode << block_size[mode] - 1;
            fread(&serial[1], sizeof (UWord8), block_size[mode] - 1, f_serial );
            frame++;
            fprintf(stderr, " Decoding frame: %ld\r", frame);
            D_IF_decode( st, serial, synth, _good_frame);
            fwrite(synth, sizeof(Word16), L_FRAME16k, f_synth);
            fflush(f_synth);
        }
        D_IF_exit(st);
        fclose(f_serial);
        fclose(f_synth);
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
