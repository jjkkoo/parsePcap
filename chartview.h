
#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts/QChartView>
//#include <QtWidgets/QRubberBand>

#include <QDebug>
//#include <QtMath>

#include "progressline.h"

QT_CHARTS_USE_NAMESPACE

typedef struct zoomInfo {
    double start;
    double end;
    double step;
    double max;
}zoomInfo;

class ChartView : public QChartView
{
    Q_OBJECT

public:
    ChartView(QChart *chart, QWidget *parent = 0);
    void setDataLength(int dataLen);
    int getDataLength(int index);
    void setZoomInfo(zoomInfo zi);
    zoomInfo getZoomInfo(int index);
    void refreshProgress(double progress);

signals:
    void updateStartPosFromClick(double posPercent);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    //bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
//    void paintEvent(QPaintEvent *event) override;

private:
    bool m_isTouching;
    QList<int> m_dataLength;
    QList<zoomInfo> zoomInfoList;

    ProgressLine *progressLine;
    int PLWidth;
};

#endif
