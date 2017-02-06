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

# Linux link to boost libraries
unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_filesystem
unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_system

# Linux link to libAirspaceConverter
unix:!macx: LIBS += -lairspaceconverter

# Windows link to boost libraries
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../boost_1_61_0/lib64/ -llibboost_filesystem-vc140-mt-1_61
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../boost_1_61_0/lib64/ -llibboost_filesystem-vc140-mt-gd-1_61
INCLUDEPATH += $$PWD/../../../../../../../boost_1_61_0
DEPENDPATH += $$PWD/../../../../../../../boost_1_61_0

# Windows link to libAirspaceConverter
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../VisualStudio/Release/x64/ -lAirspaceConverterLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../VisualStudio/Debug/x64/ -lAirspaceConverterLib
win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../VisualStudio/Release/x64/AirspaceConverterLib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../VisualStudio/Debug/x64/AirspaceConverterLib.lib

# Windows link to libzip
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/lib/x64/v140/Release/ -lzip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/lib/x64/v140/Debug/ -lzip
INCLUDEPATH += $$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/include
DEPENDPATH += $$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/include
