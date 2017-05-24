#include "mainwindow.h"
#include <QApplication>
#include <QMetaType>



int main(int argc, char *argv[])
{
    qRegisterMetaType< QList<QStringList> >("parseResult");
    qRegisterMetaType< QList<QTemporaryFile *> > ("mediaFileList");
    //QCoreApplication::addLibraryPath("./dll");
    QApplication a(argc, argv);
    a.setApplicationDisplayName("parsePcap");
    MainWindow w;
    w.show();

    return a.exec();
}
