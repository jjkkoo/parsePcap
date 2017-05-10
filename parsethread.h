#include <QThread>
#include <QDebug>
#include <QMutex>
#include <Qdatetime>

#include <stdio.h>
#define HAVE_REMOTE
#include "pcap.h"
#include "remote-ext.h"

#include <winsock2.h>

class ParseThread : public QThread
{
    Q_OBJECT

public:
    explicit ParseThread(QString jobFile, QObject *parent = 0);
    ~ParseThread();

signals:
    void resultReady(int value);

public slots:
    void stopMe();

protected:
    virtual void run()  ;

private:
    bool m_abort;
    QMutex mutex;
    QString pcapFileName;
    int packetCount;
};
