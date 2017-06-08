#ifndef ProgressLine_H
#define ProgressLine_H
#include <QWidget>
#include <QPainter>

class ProgressLine : public QWidget
{
public:
    ProgressLine(QWidget * parent);
    void setStartPos(int pos);
    void reset();
    void setPos(int pos);
    //void setWidth(int width);

protected:
    void paintEvent(QPaintEvent * event) override ;

//private slots:
//    void resizeToParent();

private:
    QWidget * parent;
    int position_current;
    int position_start;
    //QList<QRect> playHistory;
    //int position_end;
};

#endif // ProgressLine_H
