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


SOURCES += main.cpp\
        mainwindow.cpp \
    customchartmodel.cpp \
    customtablemodel.cpp \
    parsethread.cpp

HEADERS  += mainwindow.h \
    customchartmodel.h \
    customtablemodel.h \
    parsethread.h

FORMS    += mainwindow.ui
