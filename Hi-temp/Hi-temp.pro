#-------------------------------------------------
#
# Project created by QtCreator 2015-09-29T21:35:07
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Hi-temp
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    BasicExcel.cpp

HEADERS  += widget.h \
    BasicExcel.hpp

FORMS    += widget.ui

RC_FILE  += myicon.rc
