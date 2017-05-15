#include <parsethread.h>

//#define LINE_LEN 16
const char packet_filter[] = "ip and udp || ip6 and udp";
const QString parseWasSuccess("parsing seems okay");
QString parseError;
struct bpf_program fcode;
u_int netmask = 0xffffff;

int getIpv6Addr(char *output, int *output_len, u_char *input) {
    int j = 0, ret = 0 ;
    for (int i = 0; i < 8; ++i) {
        if (*(input + 2 * i) == 0)
            ret = sprintf (output + j ,"%x" ,*(input + 2 * i +1));
        else
            ret = sprintf (output + j, "%2x%02x" ,*(input + 2 * i) , *(input + 2 * i + 1));
        if (ret < 0)
            return -1;
        j = j + ret;
        if (i < 7)
            sprintf (output + j++, ":");
    }
    //*output_len = j;
    return 0;
}

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
    int res;
    int dl_len;

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
    int dl_type = pcap_datalink(fp);
    if(dl_type == DLT_EN10MB)
        dl_len = 14;
    else if (dl_type == DLT_RAW)
        dl_len = 0;
    else {
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

    pcap_close (fp);

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

    eth_header *eh;
    ip_header *ih;
    ipv6_header *ihv6;
    udp_header *uh;
    rtp_header *rh;
    rtp_ext_header *reh;
    u_int ip_len;
    u_int rtp_len;
    int rtp_ext_len;

    int currentPacketNo = 0;
    int currentProgress = 0;

    QString sourceIp, srcPort, destIp, destPort, pktDateTime, payloadType, ssrc;
    QDateTime tempDateTime;
    QSet<QByteArray> uniquePacketSet;
    QByteArray tempByteArray;
    QList<QStringList> parseResult;// = new QList<QStringList>();
    QHash<QString, parseResultInfo> parseResultDict;
    const char magicByte[] = {"\xb4\xc3\xb2\xa1"};
    char sourceIpv6Buffer[40], destIpv6Buffer[40];
    int *sourceIPv6Len = 0,  *destIpv6len = 0;
    char ip_version;

    m_parseResult->clear();
    while((res = pcap_next_ex( fp, &header, &pkt_data)) >= 0)
    {
        if (m_abort)    return;

        if (100 * ++currentPacketNo / packetCount > currentProgress) {
            emit updateProgress(++currentProgress);
        }

        /* check duplicated packet
        tempByteArray = QByteArray((char *)pkt_data, header->len);
        if (!uniquePacketSet.contains(tempByteArray)) {
            uniquePacketSet.insert(tempByteArray);*/

        /* check ip version */
        ip_version = *(pkt_data + dl_len) & 0xf0;
        if (ip_version == 0x40){
            /* retrieve ip header */
            ih = (ip_header *) (pkt_data + dl_len); //length of ethernet header
            sourceIp = QString("%1.%2.%3.%4").arg(ih->saddr.byte1).arg(ih->saddr.byte2).arg(ih->saddr.byte3).arg(ih->saddr.byte4);
            destIp = QString("%1.%2.%3.%4").arg(ih->daddr.byte1).arg(ih->daddr.byte2).arg(ih->daddr.byte3).arg(ih->daddr.byte4);
            ip_len = (ih->ver_ihl & 0xf) * 4;
        }
        else if(ip_version == 0x60){
            /* retrieve ipv6 header */
            ihv6 = (ipv6_header *) (pkt_data + dl_len); //length of ethernet header
            if (getIpv6Addr(sourceIpv6Buffer, sourceIPv6Len, ihv6->Srcv6) == 0) {
                sourceIp = QString(sourceIpv6Buffer);
            }
            if (getIpv6Addr(destIpv6Buffer, destIpv6len, ihv6->Destv6) == 0) {
                destIp = QString(destIpv6Buffer);
            }

            ip_len = 40;    //todo this is inaccurate
        }
        else {
            qDebug() << "unknown type error";
            return;
        }

        /* retrieve udp header */
        uh = (udp_header *) (pkt_data + dl_len + ip_len);

        /* assert rtp, assuming smallest amr is 7 bytes */
        if (uh->len >= 8 + sizeof(rtp_header) + 7) {
            /* retrieve rtp header */
            rh = (rtp_header *)((u_char*)uh + 8);
            if (rh->x) {
                reh = (rtp_ext_header *)((u_char*)rh + 12);
                rtp_ext_len = 32 * reh->length;
                rtp_len = 12 + 4 + rtp_ext_len + rh->cc * 32;
            }
            else {
                rtp_len = 12 + rh->cc * 32;
            }
            /* assert amr packet, check pt 96 <= pt <=127 , check srcPort and destPort > 1024 */
            if (rh->pt >= 96 and ntohs(uh->sport) > 1024 and ntohs(uh->dport) >1024) {
                /* extract information from packet */
                tempDateTime = QDateTime::fromTime_t(header->ts.tv_sec);
                pktDateTime = tempDateTime.toString("yyyy-MM-dd hh:mm:ss") + "." + QString::number(header->ts.tv_usec, 10);
                srcPort = QString("%1").arg(ntohs(uh->sport), 10).trimmed();
                destPort = QString("%1").arg(ntohs(uh->dport), 10).trimmed();
                payloadType = QString("%1").arg(rh->pt);
                ssrc = QString::number(ntohl(rh->ssrc), 16).trimmed().toUpper();

                QString tempHashKey = QString("%1;%2;%3;%4;%5").arg(sourceIp).arg(srcPort).arg(destIp).arg(destPort).arg(ssrc);
                if (parseResultDict.contains(tempHashKey)){
                    /* update depository entry */
                    parseResult[parseResultDict[tempHashKey].position][COL_last_packet_time] = pktDateTime;
                    ++parseResultDict[tempHashKey].pktCount;

                    int numWritten = parseResultDict[tempHashKey].mediaFile->write((char *)rh + rtp_len, header->len - dl_len - ip_len - 8 - rtp_len);
                    parseResultDict[tempHashKey].mediaFile->write(magicByte);
                }
                else {
                    /* create new depository entry */
                    QStringList tmpSL;
                    tmpSL << sourceIp << srcPort << destIp << destPort << pktDateTime << pktDateTime << "" << payloadType << ssrc << "" << "" << "" << "" << "";
                    parseResult.append(tmpSL);

                    QTemporaryFile *tmpFile = new QTemporaryFile("parsePcap"); //todo free memory
                    if (tmpFile->open()){

                        tmpFile->write(magicByte);
                        int numWritten = tmpFile->write((char *)rh + rtp_len, header->len - dl_len - ip_len - 8 - rtp_len);
                        tmpFile->write(magicByte);
                    }
                    parseResultInfo *pri = new parseResultInfo {parseResult.size() - 1, 1, tmpFile};    //todo free memory
                    parseResultDict.insert(tempHashKey, *pri);
                }
            }
        }
    }

    QTemporaryFile * mediaFileList[parseResultDict.size()];
    foreach (const parseResultInfo &tmppri ,parseResultDict){
        parseResult[tmppri.position][COL_pktCount] = QString("%1").arg(tmppri.pktCount);
        mediaFileList[tmppri.position] = tmppri.mediaFile;
        mediaFileList[tmppri.position]->close();
    }

    QList<QTemporaryFile *> mediaList;
    for (int i = 0; i < parseResultDict.size(); ++i){
        mediaList << mediaFileList[i];
    }
    emit parseSuccess(parseResult, mediaList);
    emit lastWords(parseWasSuccess);
    pcap_close (fp);
}

void ParseThread::stopMe()
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();
}


