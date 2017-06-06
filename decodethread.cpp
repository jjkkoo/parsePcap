#include "decodethread.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "interf_dec.h"
#include "interf_rom.h"
#include "rom_dec.h"
#include "sp_dec.h"
#include "typedef.h"

#include "dec.h"
#include "dec_acelp.h"
#include "dec_dtx.h"
#include "dec_gain.h"
#include "dec_if.h"
#include "dec_lpc.h"
#include "dec_main.h"
#include "dec_util.h"
#include "if_rom.h"
#include "typedef.h"
}

const QString decodeWasSuccess("decoding seems okay");

DecodeThread::DecodeThread(QTemporaryFile *decodeFile, int codec, QObject *parent) : QThread(parent),
    m_abort(false), decodeFile(decodeFile), codec(codec)
{
    qDebug() << "Decoder construct Thread : " << QThread::currentThreadId() << "," << "decodeFileName:" << decodeFile << "," << "codec:" << codec;
    if (codec == 0 or codec == 2) {
        paddingDataLen = 320;
        FTsilent = 8;
    }
    else if (codec == 1 or codec == 3) {
        paddingDataLen = 640;
        FTsilent = 9;
    }
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

    QTemporaryFile *decodeResult = new QTemporaryFile("temp\\decoder"); //todo free memory
    if (decodeResult->open()){
        qDebug() << "decodeResult:" << decodeResult->fileName();
    }

    unsigned int tempByteRead = 0, filesize = decodeFile->size();
    unsigned int frame = 0;
    unsigned int currentProgress = 0;
    void *st;
    unsigned char FTList[12], FTCount;    //max number of amr frame in one packet: 12
    unsigned short pktLen;                //current packet length
    unsigned char currentFTptr = 4;       //FT pointer and amr data pointer
    unsigned int bytePtr = 0;             //current byte pointer
    FILE *f_serial;                        /* File of serial bits for transmission  */
    unsigned int speechBufferLen, synthLen;

    if ((f_serial = fopen(qPrintable(decodeFile->fileName()), "rb")) == NULL)
    {
        qDebug() << "error open input file:" << decodeFile->fileName();
        return;
    }

    if (codec == 0 or codec == 2) {
        speechBufferLen = 32;
        synthLen = 160;
        st = Decoder_Interface_init();
    }
    else if (codec == 1 or codec == 3) {
        speechBufferLen = 61;
        synthLen = 320;
        st = D_IF_init();
    }

    short synth[synthLen];              /* Buffer for speech */
    unsigned char serial[speechBufferLen], pktBuffer[speechBufferLen * 12];

    while(fread(&pktLen, sizeof(unsigned short), 1, f_serial)) {
        if (m_abort)    return;
        tempByteRead += (2 + pktLen);
        //qDebug() << tempByteRead << " " << filesize;
        if (100 * ++tempByteRead / filesize > currentProgress)
            emit updateProgress(currentProgress++);
        //qDebug() << pktLen;
        FTCount = 0;
        bytePtr = 0;
        currentFTptr = 4;
        if (fread(pktBuffer, sizeof (unsigned char), pktLen, f_serial )!=pktLen){     //read whole packet
            qWarning() << "error reading packets";
            break;
        }
        /* get all FT into an array, max len 12 */
        while (FTCount < 12 and bytePtr < pktLen) {
            if (codec == 0 or codec == 1) {
                if(currentFTptr > 3 and currentFTptr < 7) {
                    FTList[FTCount++] =  ((pktBuffer[bytePtr] << (currentFTptr - 3)) & 0x0f) | ( pktBuffer[bytePtr + 1] >> (11 - currentFTptr));
                }
                else if (currentFTptr <= 3) {
                    FTList[FTCount++] = ( pktBuffer[bytePtr] >> (3 - currentFTptr)) & 0x0f;
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
                            FTList[FTCount++] = ( (pktBuffer[bytePtr] << (currentFTptr - 3)) & 0x0f ) | ( pktBuffer[bytePtr + 1] >> (11 - currentFTptr));
                        }
                        currentFTptr += 6;
                    }
                    currentFTptr %= 8;
                    ++bytePtr;
                }
            }
            else if (codec == 2 or codec == 3) {
                FTList[FTCount++] = (pktBuffer[++bytePtr] >> 3) & 0x0f;
                if ((pktBuffer[bytePtr] & 0x80) == 0) {
                    break;
                }
            }
        }
        if (codec == 0 or codec == 1) {    //for bandwidth efficient
            currentFTptr += 6;
            bytePtr = bytePtr + currentFTptr / 8;
            currentFTptr %= 8;
        }
        else if (codec == 2 or codec == 3) {    //for octet aligned
            currentFTptr = 0;
            ++bytePtr;
        }
        //qDebug() << "FTCount:" << FTCount;

        /* for each amr data in each pkt, decode follow FT indicate */
        for (int i=0; i<FTCount; ++i) {
            //qDebug() << FTList[i];
            if (FTList[i] < FTsilent) {
                serial[0] = FTList[i] << 3 | 0x4;
                int serialByteLen, serialByteMod;
                if (codec == 1 or codec == 3) {
                    serialByteLen = qCeil(amr_wb_nob[FTList[i]] / 8.0);    //useful amr data len in bytes
                    serialByteMod = amr_wb_nob[FTList[i]] % 8;             //last amr data byte, bit len
                }
                else if (codec == 0 or codec == 2) {
                    serialByteLen = qCeil(amr_nb_nob[FTList[i]] / 8.0);    //useful amr data len in bytes
                    serialByteMod = amr_nb_nob[FTList[i]] % 8;             //last amr data byte, bit len
                }
                for( int j = 1; j <= serialByteLen and bytePtr < pktLen; ++j, ++bytePtr) {
                    if (j < serialByteLen) {
                        if (currentFTptr == 0) {
                            serial[j] = pktBuffer[bytePtr];
                        }
                        else {
                            serial[j] = pktBuffer[bytePtr]<<currentFTptr | pktBuffer[bytePtr + 1]>>(8 - currentFTptr);
                        }
                    }
                    else {    //last byte
                        if (currentFTptr == 0) {
                            serial[j] = pktBuffer[bytePtr] & 0xff << (8 - serialByteMod);
                        }
                        else {
                            serial[j] = pktBuffer[bytePtr]<<currentFTptr;// & 0xff<<(8 - serialByteMod);
                        }
                    }
                }
                if (codec == 0 or codec == 1) {    //for bandwidth efficient
                    currentFTptr = (currentFTptr + serialByteMod) % 8;
                    if (currentFTptr != 0)    --bytePtr;
                }
                frame++;
                if (codec == 0 or codec == 2)
                    Decoder_Interface_Decode(st, serial, synth, 0);
                else if (codec == 1 or codec ==3)
                    D_IF_decode( st, serial, synth, _good_frame);

                decodeResult->write((char *)synth, synthLen * 2);
            }
        else if (FTList[i] == FTsilent) {
            decodeResult->write(QByteArray(paddingDataLen * 8 , 0));
            }
        else if (FTList[i] > FTsilent) {
            decodeResult->write(QByteArray(paddingDataLen , 0));
            }
        }
    }

    if (codec == 0 or codec == 2)
        Decoder_Interface_exit(st);
    else if (codec == 1 or codec ==3)
        D_IF_exit(st);

    fclose(f_serial);
    decodeResult->close();
    qDebug() << decodeResult->fileName();
    qDebug() << "total frame :" << frame;

    //qDebug() << QDateTime::currentDateTime();
    emit decodeSuccess(decodeResult);
    emit lastWords(decodeWasSuccess);
}

void DecodeThread::stopMe()
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();
}
