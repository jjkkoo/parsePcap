
#include "chartview.h"
#include <QtGui/QMouseEvent>

ChartView::ChartView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent), m_isTouching(false), m_dataLength{0}, zoomInfoList{{0,0,0,0}},
            progressLine(new ProgressLine(this))
{
    //setRubberBand(QChartView::RectangleRubberBand);

}

void ChartView::setZoomInfo(zoomInfo zi) {
    zoomInfoList[0] = zi;
}

zoomInfo ChartView::getZoomInfo(int index) {
    return zoomInfoList.at(index);
}

void ChartView::resizeEvent(QResizeEvent *event){
    chart()->resize(event->size());
    progressLine->setGeometry(chart()->plotArea().toRect());
    PLWidth = chart()->plotArea().toRect().width();
    progressLine->update();
}

void ChartView::refreshProgress(double progress)
{
    if (progress > 1)    progress = 1;
        //qDebug() << progress << " " << chart()->plotArea().toRect().width();
        progressLine->setPos((zoomInfoList.at(0).max * progress - zoomInfoList.at(0).start) * PLWidth / zoomInfoList.at(0).step);
        progressLine->update();
}

void ChartView::finishProgress()
{
    progressLine->setPos(PLWidth);
    progressLine->update();
}

void ChartView::setDataLength(int dataLen) {
    m_dataLength[0] = dataLen;
}

int ChartView::getDataLength(int index){
    return m_dataLength.at(index);
}

//bool ChartView::viewportEvent(QEvent *event)
//{
//    if (event->type() == QEvent::TouchBegin) {
//        // By default touch events are converted to mouse events. So
//        // after this event we will get a mouse event also but we want
//        // to handle touch events as gestures only. So we need this safeguard
//        // to block mouse events that are actually generated from touch.
//        m_isTouching = true;

//        // Turn off animations when handling gestures they
//        // will only slow us down.
//        chart()->setAnimationOptions(QChart::NoAnimation);
//    }
//    return QChartView::viewportEvent(event);
//}

void ChartView:: wheelEvent(QWheelEvent *event)
{
    //QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;
    int tempPos = progressLine->mapFromGlobal(event->globalPos()).x();//current mouse posion in plot
    //qDebug() << tempPos << 1.0 * tempPos/PLWidth *zoomInfoList[0].step + zoomInfoList[0].start << PLWidth << zoomInfoList[0].start;
    if (numDegrees.y() > 0) {                                    //zoom in
        if (zoomInfoList.at(0).step > 0.002) {                       //least step 20ms
            zoomInfoList[0].start = zoomInfoList[0].start + 0.2 * tempPos / PLWidth * zoomInfoList[0].step;
            zoomInfoList[0].step = zoomInfoList[0].step * 0.8;    //zoom in 80%
            zoomInfoList[0].end = zoomInfoList[0].start + zoomInfoList[0].step;
            //qDebug() << zoomInfoList[0].start << 1.0 * tempPos/PLWidth *zoomInfoList[0].step + zoomInfoList[0].start;
            chart()->axisX()->setRange(QVariant(zoomInfoList.at(0).start), QVariant(zoomInfoList.at(0).end));

            if (zoomInfoList.at(0).step < 1 and zoomInfoList.at(0).step > 0.8)
                emit showAllOneSecondPoints();
            if (zoomInfoList.at(0).step < 0.05 and zoomInfoList.at(0).step > 0.04)
                dynamic_cast<QLineSeries*>(chart()->series()[0])->setPointsVisible(true);
        }
    }
    else {                                                       //zoom out
        if (zoomInfoList.at(0).step < zoomInfoList.at(0).max) {  //make sure step less than max
            zoomInfoList[0].start = zoomInfoList[0].start + (-0.25) * tempPos / PLWidth * zoomInfoList[0].step;
            if (zoomInfoList[0].start < 0)    zoomInfoList[0].start = 0;
            zoomInfoList[0].step = zoomInfoList[0].step / 0.8;    //zoom out 80%
            if (zoomInfoList[0].step > zoomInfoList.at(0).max)    zoomInfoList[0].step = zoomInfoList.at(0).max;
            zoomInfoList[0].end = zoomInfoList[0].start + zoomInfoList[0].step;
            chart()->axisX()->setRange(QVariant(zoomInfoList.at(0).start), QVariant(zoomInfoList.at(0).end));
        }
        else {
            if (zoomInfoList.at(0).start != 0) {
                zoomInfoList[0].start = 0;
                zoomInfoList[0].step = zoomInfoList.at(0).max;
                zoomInfoList[0].end = zoomInfoList[0].start + zoomInfoList[0].step;
                chart()->axisX()->setRange(QVariant(zoomInfoList.at(0).start), QVariant(zoomInfoList.at(0).end));
            }
        }
        if (zoomInfoList.at(0).step > 1 and zoomInfoList.at(0).step < 1.25)
            emit showAllSampledPoints();
        if (zoomInfoList.at(0).step > 0.05 and zoomInfoList.at(0).step < 0.0625)
            dynamic_cast<QLineSeries*>(chart()->series()[0])->setPointsVisible(false);
    }
//    qDebug() << chart()->plotArea();
//    qDebug() << this->mapToGlobal(chart()->plotArea().topLeft().toPoint());
//    qDebug() << zoomInfoList.at(0).start << ";" << zoomInfoList.at(0).end << ";" << zoomInfoList.at(0).step << ";" << zoomInfoList.at(0).max;
    //this->se
    event->accept();
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
//    qDebug() << chart()->plotArea();
//    qDebug() << event->x() << ":" << event->y() << ":" << event->globalX() << ":" << event->globalY();

    if (m_isTouching)
        return;
    if (event->button() == Qt::LeftButton) {
        int tempPos = progressLine->mapFromGlobal(event->globalPos()).x();
        progressLine->setStartPos(tempPos);
        progressLine->update();
        emit updateStartPosFromClick(1.0 * tempPos / PLWidth);
    }
    QChartView::mousePressEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isTouching)
        return;
    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isTouching)
        m_isTouching = false;

    // Because we disabled animations when touch event was detected
    // we must put them back on.
    chart()->setAnimationOptions(QChart::SeriesAnimations);

    QChartView::mouseReleaseEvent(event);
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

//void ChartView::paintEvent(QPaintEvent *event)
//{
//    qDebug() << chart()->plotArea();
//    qDebug() << this->mapToGlobal(chart()->plotArea().topLeft().toPoint());
//    qDebug() << zoomInfoList.at(0).start << ";" << zoomInfoList.at(0).end << ";" << zoomInfoList.at(0).step << ";" << zoomInfoList.at(0).max;
//    qDebug() << this->mapToGlobal(geometry().topLeft());
//    QChartView::paintEvent(event);
//}

void ChartView::resizeProgressLine()
{
    PLWidth = chart()->plotArea().toRect().width();
    progressLine->setGeometry(chart()->plotArea().toRect());
    progressLine->update();
}
