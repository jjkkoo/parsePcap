#include <QThread>
#include <QDebug>

class ParseThread : public QThread
{
    Q_OBJECT

public:
    explicit ParseThread(QObject *parent = 0);

signals:
    void resultReady(int value);

public slots:
    void stopMe();

protected:
    virtual void run()  ;

private:
    bool m_abort;

};
