#include <QThread>
#include <QDebug>
#include <parsethread.h>

ParseThread::ParseThread(QObject *parent) : QThread(parent)
{
    qDebug() << "Worker Thread : " << QThread::currentThreadId();
}

void ParseThread::run()
{
    qDebug() << "Worker Run Thread : " << QThread::currentThreadId();
    int nValue = 0;
    while (nValue < 100)
    {
        msleep(100);
        ++nValue;

        emit resultReady(nValue);
    }
}

