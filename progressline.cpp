#include "progressline.h"

ProgressLine::ProgressLine(QWidget * parent = nullptr) : QWidget{parent},
    position_current(0), position_start(0)//, playHistory(QList<QRect>())//, position_end(0)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void ProgressLine::paintEvent(QPaintEvent * event) {
    QPainter pt(this);
    pt.setPen(Qt::red);
    pt.fillRect(QRect(0, 0, position_current, height()), {0x87, 0xce, 0xeb, 0x30});
//    foreach (const QRect& r, playHistory) {
//        pt.fillRect(r, {0x87, 0xce, 0xeb, 0x30});
//    }
//    pt.fillRect(QRect(position_start, 0, position_current, height()), {0x87, 0xce, 0xeb, 0x30}); // 	sky blue #87CEEB
    pt.drawLine(position_start, 0, position_start, height());
}

//void ProgressLine::setWidth(int width){
//    position_end = width;
//}

void ProgressLine::setStartPos(int pos) {
    //playHistory.append(QRect(position_start, 0, position_current, height()));
    position_start = pos;
}

void ProgressLine::reset() {
    position_start = 0;
    //playHistory.clear();
    position_current = 0;
}

void ProgressLine::setPos(int pos) {
    position_current = pos;
}
