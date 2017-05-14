#include <QThread>
#include <QDebug>
#include <QMutex>
#include <Qdatetime>
#include <QTemporaryFile>

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

/* mac addr */
typedef struct mac_address{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
    u_char byte5;
    u_char byte6;
}mac_address;

/* 14 bytes ethernet heeader */
typedef struct eth_header{
    mac_address daddr;
    mac_address saddr;
    u_short type;
}eth_header;

/* 4 bytes IP address */
typedef struct ip_address{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header{
    u_char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
    u_char  tos;            // Type of service
    u_short tlen;           // Total length
    u_short identification; // Identification
    u_short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
    u_char  ttl;            // Time to live
    u_char  proto;          // Protocol
    u_short crc;            // Header checksum
    ip_address  saddr;      // Source address
    ip_address  daddr;      // Destination address
    u_int   op_pad;         // Option + Padding
}ip_header;

/* IPv6 header */
typedef struct ipv6_header {
    u_char ver_tf;
    u_char traffic;
    u_short label;
    u_char length[2];
    u_char next_header;
    u_char limits;
    u_char Srcv6[16];
    u_char Destv6[16];
}ipv6_header;

/* UDP header*/
typedef struct udp_header{
    u_short sport;          // Source port
    u_short dport;          // Destination port
    u_short len;            // Datagram length
    u_short crc;            // Checksum
}udp_header;

/* RTP header*/
typedef struct rtp_header{
    u_char cc:4;
    u_char x:1;
    u_char p:1;
    u_char v:2;
    u_char pt:7;
    u_char m:1;
    u_short sequence_number;
    u_int timestamp;
    u_int ssrc;
}rtp_header;

/* rtp extension header */
typedef struct rtp_ext_header{
    u_short profile;
    u_short length;
}rtp_ext_header;

/* miscellaneous infor for parse result */
typedef struct parseResultInfo{
    int position;
    int pktCount;
    QTemporaryFile *mediaFile;
}parseResultInfo;

class ParseThread : public QThread
{
    Q_OBJECT

public:
    explicit ParseThread(QString jobFile, QObject *parent = 0);
    ~ParseThread();

signals:
    void updateProgress(int value);
    void parseSuccess(QList<QStringList> parseResult, QStringList fileNameList);

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
