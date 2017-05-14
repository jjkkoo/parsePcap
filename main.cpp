#include "mainwindow.h"
#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    qRegisterMetaType< QList<QStringList> >("parseResult");
    qRegisterMetaType< QList<QTemporaryFile *> > ("mediaFileList");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
