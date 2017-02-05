#-------------------------------------------------
#
# Project created by QtCreator 2017-01-29T12:24:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AirspaceConverterQt
TEMPLATE = app

CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    ../src/Airfield.h \
    ../src/Airspace.h \
    ../src/AirspaceConverter.h \
    ../src/CUPreader.h \
    ../src/Geometry.h \
    ../src/KMLwriter.h \
    ../src/OpenAIPreader.h \
    ../src/OpenAir.h \
    ../src/PFMwriter.h \
    ../src/RasterMap.h \
    ../src/Waypoint.h \
    aboutdialog.h

FORMS    += mainwindow.ui \
    aboutdialog.ui

unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_filesystem
unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_system

INCLUDEPATH += $$PWD/../Release
DEPENDPATH += $$PWD/../Release

unix:!macx: LIBS += -lairspaceconverter

