
#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts/QChartView>
#include <QtWidgets/QRubberBand>

#include <QDebug>

QT_CHARTS_USE_NAMESPACE


typedef struct zoomInfo {
    int start;
    int end;
    int step;
    int max;
}zoomInfo;

class ChartView : public QChartView
{
public:
    ChartView(QChart *chart, QWidget *parent = 0);
//    void setDataLength(int dataLen);
//    int getDataLength(int index);
    void setZoomInfo(zoomInfo zi);
    zoomInfo getZoomInfo(int index);


protected:
    void wheelEvent(QWheelEvent *event);
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);


private:
    bool m_isTouching;
    QList<int> m_dataLength;
    QList<zoomInfo> zoomInfoList;
};

#endif
