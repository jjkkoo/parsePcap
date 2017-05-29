#-------------------------------------------------
#
# Project created by QtCreator 2017-04-29T02:17:22
#
#-------------------------------------------------

QT       += core gui
QT       += charts
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = parsePcap
TEMPLATE = app

INCLUDEPATH += $$PWD\WpdPack\Include
INCLUDEPATH += $$PWD\amr_nb\include
INCLUDEPATH += $$PWD\amr_wb\include
LIBS += $$PWD\WpdPack\Lib\libwpcap.a
LIBS += $$PWD\WpdPack\Lib\libpacket.a
LIBS += $$PWD\amr_nb\lib\amr_nb32.a
LIBS += $$PWD\amr_wb\lib\amr_wb32.a
LIBS += libws2_32

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    customtablemodel.cpp \
    parsethread.cpp \
    comboboxdelegate.cpp \
    customchart.cpp \
    chartview.cpp \
    decodethread.cpp \
    tabdialog.cpp \
    dialogmodel.cpp \
    progressline.cpp

HEADERS  += mainwindow.h \
    customtablemodel.h \
    parsethread.h \
    comboboxdelegate.h \
    customchart.h \
    chartview.h \
    decodethread.h \
    tabdialog.h \
    dialogmodel.h \
    wavefile.h \
    progressline.h

