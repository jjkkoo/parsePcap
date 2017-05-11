#include "mainwindow.h"
#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    qRegisterMetaType< QList<QStringList> >();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
