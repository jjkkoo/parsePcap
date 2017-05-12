#include <QThread>
#include <QDebug>
#include <QMutex>
#include <Qdatetime>

#include <stdio.h>
#define HAVE_REMOTE
#include "pcap.h"
#include "remote-ext.h"

#include <winsock2.h>

#ifndef COLUMNS_H
#define COLUMNS_H
enum Columns
{
    COL_source_ip,
    COL_srcPort,
    COL_dest_ip,
    COL_destPort,
    COL_first_packet_time,
    COL_last_packet_time,
    COL_pktCount,
    COL_PT,
    COL_SSRC,
    COL_codec,
    COL_Lost,
    COL_Dup,
    COL_WrongSeq,
    COL_MaxDelta
};
#endif

class ParseThread : public QThread
{
    Q_OBJECT

public:
    explicit ParseThread(QString jobFile, QObject *parent = 0);
    ~ParseThread();

signals:
    void resultReady(int value);
    void parseSuccess(QList<QStringList> parseResult);

public slots:
    void stopMe();

protected:
    virtual void run();

private:
    bool m_abort;
    QMutex mutex;
    QString pcapFileName;
    int packetCount;
    QList<QStringList>* m_parseResult;
};
