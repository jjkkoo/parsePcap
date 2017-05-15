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

INCLUDEPATH += c:\WpdPack\Include
LIBS += c:\WpdPack\Lib\libwpcap.a
LIBS += c:\WpdPack\Lib\libpacket.a
LIBS += libws2_32

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    customtablemodel.cpp \
    parsethread.cpp \
    comboboxdelegate.cpp \
    customchart.cpp \
    chartview.cpp

HEADERS  += mainwindow.h \
    customtablemodel.h \
    parsethread.h \
    comboboxdelegate.h \
    customchart.h \
    chartview.h

