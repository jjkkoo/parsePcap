#include <QThread>
#include <QDebug>
#include <QMutex>

class ParseThread : public QThread
{
    Q_OBJECT

public:
    explicit ParseThread(QObject *parent = 0);
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
};
