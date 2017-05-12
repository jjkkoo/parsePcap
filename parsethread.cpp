#include <parsethread.h>

#define LINE_LEN 16
char packet_filter[] = "udp";
struct bpf_program fcode;
u_int netmask = 0xffffff;

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

ParseThread::ParseThread(QString jobFile, QObject *parent) : QThread(parent), pcapFileName(jobFile), packetCount(0)
    ,m_parseResult(new QList<QStringList>())
{
    qDebug() << "Worker Thread : " << QThread::currentThreadId() << "," << "pcapFileName:" << pcapFileName;
    m_abort = false;
}

ParseThread::~ParseThread()
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();

    wait();
}

void ParseThread::run()
{
    qDebug() << "Worker Run Thread : " << QThread::currentThreadId();

    pcap_t *fp;
    char errbuf[PCAP_ERRBUF_SIZE];
    char source[PCAP_BUF_SIZE];
    struct pcap_pkthdr *header;
    const u_char *pkt_data;
    //u_int i=0;
    int res;

    /* Create the source string according to the new WinPcap syntax */
    if ( pcap_createsrcstr( source,         // variable that will keep the source string
                            PCAP_SRC_FILE,  // we want to open a file
                            NULL,           // remote host
                            NULL,           // port on the remote host
                            qPrintable(pcapFileName),    // name of the file we want to open
                            errbuf          // error buffer
                            ) != 0)
    {
        fprintf(stderr,"\nError creating a source string\n");
        return;
    }

    /* Open the capture file */
    if ( (fp= pcap_open(source,         // name of the device
                        65536,          // portion of the packet to capture
                                        // 65536 guarantees that the whole packet will be captured on all the link layers
                         PCAP_OPENFLAG_PROMISCUOUS,     // promiscuous mode
                         1000,              // read timeout
                         NULL,              // authentication on the remote machine
                         errbuf         // error buffer
                         ) ) == NULL)
    {
        fprintf(stderr,"\nUnable to open the file %s.\n", source);
        return;
    }

    /* Check the link layer. We support only Ethernet for simplicity. */
    if(pcap_datalink(fp) != DLT_EN10MB)
    {
        fprintf(stderr,"\nThis program works only on Ethernet networks.\n");
        return;
    }

    //compile the filter
    if (pcap_compile(fp, &fcode, packet_filter, 1, netmask) <0 )
    {
        fprintf(stderr,"\nUnable to compile the packet filter. Check the syntax.\n");
        /* Free the device list */
        return;
    }

    //set the filter
    if (pcap_setfilter(fp, &fcode)<0)
    {
        fprintf(stderr,"\nError setting the filter.\n");
        /* Free the device list */
        return;
    }

    /* Retrieve the packets from the file */
    while((res = pcap_next_ex( fp, &header, &pkt_data)) >= 0)
    {
        ++packetCount;
    }
    qDebug() << "packetCount:" << packetCount;



    /* Open the capture file */
    if ( (fp= pcap_open(source,         // name of the device
                        65536,          // portion of the packet to capture
                                        // 65536 guarantees that the whole packet will be captured on all the link layers
                         PCAP_OPENFLAG_PROMISCUOUS,     // promiscuous mode
                         1000,              // read timeout
                         NULL,              // authentication on the remote machine
                         errbuf         // error buffer
                         ) ) == NULL)
    {
        fprintf(stderr,"\nUnable to open the file %s.\n", source);
        return;
    }

    /* Check the link layer. We support only Ethernet for simplicity. */
    if(pcap_datalink(fp) != DLT_EN10MB)
    {
        fprintf(stderr,"\nThis program works only on Ethernet networks.\n");
        return;
    }

    //compile the filter
    if (pcap_compile(fp, &fcode, packet_filter, 1, netmask) <0 )
    {
        fprintf(stderr,"\nUnable to compile the packet filter. Check the syntax.\n");
        /* Free the device list */
        return;
    }

    //set the filter
    if (pcap_setfilter(fp, &fcode)<0)
    {
        fprintf(stderr,"\nError setting the filter.\n");
        /* Free the device list */
        return;
    }

    ip_header *ih;
    udp_header *uh;
    rtp_header *rh;
    rtp_ext_header *reh;
    u_int ip_len;
    u_int rtp_len;
    int rtp_ext_len;
    //u_short sport,dport;
    //time_t local_tv_sec;

    int currentPacketNo = 0;
    int currentProgress = 0;
    m_parseResult->clear();
    QString sourceIp, srcPort, destIp, destPort, payloadType, ssrc;
    QDateTime tempDateTime;
    QSet<QByteArray> uniquePacketSet;
    QByteArray tempByteArray;
    QList<QStringList> parseResult;// = new QList<QStringList>();
    QHash<QString, int> parseResultDict;

    while((res = pcap_next_ex( fp, &header, &pkt_data)) >= 0)
    {
        if (m_abort)    return;

        if (100 * ++currentPacketNo / packetCount > currentProgress) {
            emit updateProgress(++currentProgress);
        }

        /* check duplicated packet */
        tempByteArray = QByteArray((char *)pkt_data, header->len);
        if (!uniquePacketSet.contains(tempByteArray)) {
            uniquePacketSet.insert(tempByteArray);

        /* retireve the position of the ip header */
        ih = (ip_header *) (pkt_data + 14); //length of ethernet header

        /* retireve the position of the udp header */
        ip_len = (ih->ver_ihl & 0xf) * 4;
        uh = (udp_header *) ((u_char*)ih + ip_len);

        /* assert rtp/amr packet, assuming smallest amr is 7 bytes */
        if (uh->len >= 8 + sizeof(rtp_header) + 7) {
            rh = (rtp_header *)((u_char*)uh + 8);
            if (rh->x) {
                reh = (rtp_ext_header *)((u_char*)rh + 12);
                rtp_ext_len = 32 * reh->length;
                rtp_len = 12 + 4 + rtp_ext_len + rh->cc * 32;
            }
            else {
                rtp_len = 12 + rh->cc * 32;
            }
            tempDateTime = QDateTime::fromTime_t(header->ts.tv_sec);
            sourceIp = QString("%1.%2.%3.%4").arg(ih->saddr.byte1).arg(ih->saddr.byte2).arg(ih->saddr.byte3).arg(ih->saddr.byte4);
            destIp = QString("%1.%2.%3.%4").arg(ih->daddr.byte1).arg(ih->daddr.byte2).arg(ih->daddr.byte3).arg(ih->daddr.byte4);
            srcPort = QString("%1").arg(ntohs(uh->sport), 10).trimmed();
            destPort = QString("%1").arg(ntohs(uh->dport), 10).trimmed();
            payloadType = QString("%1").arg(rh->pt);
            ssrc = QString::number(ntohl(rh->ssrc), 16).trimmed().toUpper();
/*
            qDebug() << "pktTime:" << tempDateTime.toString("yyyy-MM-dd hh:mm:ss") + "." + QString::number(header->ts.tv_usec, 10);
            qDebug() << "sourceIp:" << sourceIp;
            qDebug() << "destIp:" << destIp;
            qDebug() << "srcPort:" << srcPort;
            qDebug() << "destPort:" << destPort;
            qDebug() << "payloadType:" << payloadType;
            qDebug() << "ssrc:" << ssrc;
*/
            foreach (const QStringList& tmpSL, parseResult) {

            }
        }
    }

    }

    QList<QStringList> fakeList;
    for(int i = 0; i < 14; ++i){
        QStringList fakeString;
        for(int j = 0; j < 14; ++j)
            fakeString << QString("fake%1%2").arg(i).arg(j);
        fakeList.append(fakeString);
    }
    emit parseSuccess(fakeList);
}

void ParseThread::stopMe()
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();
}


