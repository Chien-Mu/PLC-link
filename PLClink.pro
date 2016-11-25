#-------------------------------------------------
#
# Project created by QtCreator 2016-11-16T13:57:08
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PLClink
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    plc.cpp

HEADERS  += mainwindow.h \
    plc.h

FORMS    += mainwindow.ui
