#include "mainwindow.h"
#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    //QCoreApplication::addLibraryPath("./dll");
    qRegisterMetaType< QList<QStringList> >("parseResult");
    qRegisterMetaType< QList<QTemporaryFile *> > ("mediaFileList");
    QApplication a(argc, argv);
    a.setApplicationDisplayName("parsePcap");
    MainWindow w;
    w.show();

    return a.exec();
}
