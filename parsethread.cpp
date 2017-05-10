#include <parsethread.h>

ParseThread::ParseThread(QObject *parent) : QThread(parent)
{
    qDebug() << "Worker Thread : " << QThread::currentThreadId();
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
    int nValue = 0;
    while (nValue < 100)
    {
        if (m_abort)    return;
        msleep(100);
        ++nValue;

        emit resultReady(nValue);
    }
}

void ParseThread::stopMe()
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();
}


