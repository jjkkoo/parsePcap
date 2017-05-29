#include "progressline.h"

ProgressLine::ProgressLine(QWidget * parent = nullptr) : QWidget{parent},
    position_current(0), position_start(0), position_end(0)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void ProgressLine::paintEvent(QPaintEvent * event) {
    QPainter{this}.fillRect(QRect(0, 0, position_current, height()), {0x87, 0xce, 0xeb, 0x30}); // 	sky blue #87CEEB

}

void ProgressLine::setWidth(int width){
    position_end = width;
}

void ProgressLine::setPosition(int pos) {
    position_current = pos;
}
