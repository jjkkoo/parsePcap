
#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>

#include <QDebug>

QT_CHARTS_USE_NAMESPACE

//![1]
class ChartView : public QChartView
//![1]
{
public:
    ChartView(QChart *chart, QWidget *parent = 0);
    void setDataLength(int dataLen);
    int getDataLength(int index);

//![2]
protected:
    void wheelEvent(QWheelEvent *event);
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
//![2]

private:
    bool m_isTouching;
    QList<int> m_dataLength;
};

#endif
