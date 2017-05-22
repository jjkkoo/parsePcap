#ifndef DecodeThread_H
#define DecodeThread_H
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include <bitset>
#include <string>

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

#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QTemporaryFile>

#ifndef CODEC_H
#define CODEC_H
enum codecColumns
{
    COL_amr,
    COL_amr_wb,
    COL_amr_octet_align,
    COL_amr_wb_octet_align,
    COL_h264,
    COL_DTMF,
    COL_EVS
};
#endif

typedef struct toc{
    unsigned char fBit:1;
    unsigned char FT:4;
    unsigned char qBit:1;
}toc;

typedef struct amr_header{
    unsigned char cmr:4;
    toc tocList;
    unsigned char * frameData;
}amr_header;

class DecodeThread : public QThread
{
    Q_OBJECT
public:
    explicit DecodeThread(const QString &decodeFile, int codec, QObject *parent = 0);
    ~DecodeThread();

signals:
    void updateProgress(int value);
    void decodeSuccess(QTemporaryFile * decodeResult);
    void lastWords(const QString & lastWords);

public slots:
    void stopMe();

protected:
    virtual void run();

private:
    bool m_abort;
    QMutex mutex;
    const QString decodeFile;
    int codec;
    //QTemporaryFile * decodeResult;
};

#endif // DecodeThread_H