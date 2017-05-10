#include <QThread>

class ParseThread : public QThread
{
    Q_OBJECT

public:
    explicit ParseThread(QObject *parent = 0);


protected:
    virtual void run()  ;

signals:
    void resultReady(int value);
};
