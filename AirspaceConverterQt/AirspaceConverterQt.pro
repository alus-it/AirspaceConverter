#============================================================================
# AirspaceConverter
# Since       : 14/6/2016
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : http://www.alus.it/AirspaceConverter
# Repository  : https://github.com/alus-it/AirspaceConverter.git
# Copyright   : (C) 2016-2018 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This Qt project was created on: 2017-01-29T12:24:53
# This source file is part of AirspaceConverter project
#============================================================================

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = airspaceconverter-gui
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11

RESOURCES += \
    resources.qrc

SOURCES += main.cpp\
    mainwindow.cpp \
    aboutdialog.cpp \
    limitsdialog.cpp

HEADERS  += mainwindow.h \
    aboutdialog.h \
    limitsdialog.h

FORMS    += mainwindow.ui \
    aboutdialog.ui \
    limitsdialog.ui

# Include headers of libAirspaceConverter
INCLUDEPATH += $$PWD/../src/
DEPENDPATH += $$PWD/../src/


## Linux libraries

# libAirspaceConverter
unix:!macx: LIBS += -L$$PWD/../Release/ -lairspaceconverter

# Boost libraries
unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_filesystem
unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_system
unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_locale

# Zip library
unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lzip


## Windows libraries on 64 bit
win32:contains(QMAKE_HOST.arch, x86_64) {

    # libAirspaceConverter
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../VisualStudio/Release/x64/ -lAirspaceConverterLib
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../VisualStudio/Debug/x64/ -lAirspaceConverterLib
    win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../VisualStudio/Release/x64/AirspaceConverterLib.lib
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../VisualStudio/Debug/x64/AirspaceConverterLib.lib

    # Boost libraries

    # Boost 1.61.0
    win32:CONFIG(release, debug|release): LIBS += -LC:/boost_1_61_0/lib64/ -llibboost_filesystem-vc140-mt-1_61
    else:win32:CONFIG(debug, debug|release): LIBS += -LC:/boost_1_61_0/lib64/ -llibboost_filesystem-vc140-mt-gd-1_61
    INCLUDEPATH += C:/boost_1_61_0
    DEPENDPATH += C:/boost_1_61_0

    # Boost 1.65.1
    #win32:CONFIG(release, debug|release): LIBS += -LC:/boost_1_65_1/lib32-msvc-14.1/ -llibboost_filesystem-vc141-mt-1_65_1
    #else:win32:CONFIG(debug, debug|release): LIBS += -LC:/boost_1_65_1/lib32-msvc-14.1/ -llibboost_filesystem-vc141-mt-gd-1_65_1
    #INCLUDEPATH += C:/boost_1_65_1
    #DEPENDPATH += C:/boost_1_65_1

    # libzip
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/lib/x64/v140/Release/ -lzip
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/lib/x64/v140/Debug/ -lzip
    INCLUDEPATH += $$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/include
    DEPENDPATH += $$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/include

## Windows libraries on 32 bit
} else {

    # Windows link to libAirspaceConverter
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../VisualStudio/Release/Win32/ -lAirspaceConverterLib
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../VisualStudio/Debug/Win32/ -lAirspaceConverterLib
    win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../VisualStudio/Release/Win32/AirspaceConverterLib.lib
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../VisualStudio/Debug/Win32/AirspaceConverterLib.lib

    # Windows link to boost libraries

    # Boost 1.61.0
    win32:CONFIG(release, debug|release): LIBS += -LC:/boost_1_61_0/lib32/ -llibboost_filesystem-vc140-mt-1_61
    else:win32:CONFIG(debug, debug|release): LIBS += -LC:/boost_1_61_0/lib32/ -llibboost_filesystem-vc140-mt-gd-1_61
    INCLUDEPATH += C:/boost_1_61_0
    DEPENDPATH += C:/boost_1_61_0

    # Boost 1.65.1
    #win32:CONFIG(release, debug|release): LIBS += -LC:\boost_1_65_1\lib64-msvc-14.1\ -llibboost_filesystem-vc141-mt-1_65_1
    #else:win32:CONFIG(debug, debug|release): LIBS += -LC:\boost_1_65_1/lib64-msvc-14.1\ -llibboost_filesystem-vc141-mt-gd-1_65_1
    #INCLUDEPATH += C:\boost_1_65_1
    #DEPENDPATH += C:\boost_1_65_1

    # Windows link to libzip
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/lib/Win32/v140/Release/ -lzip
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/lib/Win32/v140/Debug/ -lzip
    INCLUDEPATH += $$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/include
    DEPENDPATH += $$PWD/../VisualStudio/packages/libzip.1.1.2.7/build/native/include
}
