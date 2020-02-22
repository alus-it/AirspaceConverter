#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 13/2/2020
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2020 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Make sure that we are on macOS
if [ "$(uname)" != "Darwin" ]; then
	echo "ERROR: this script is only for macOS ..."
	exit 1
fi

# First of all: build if necessary
cd ..
./build.sh
cd macOS

# Clean older distribution files
./clean.sh

# Start building application bundle
echo "Building AirspaceConverter.app application bundle ..."
mkdir AirspaceConverter.app
cd AirspaceConverter.app
mkdir Contents
cd Contents

# Copy Info.plist and Pkginfo
cp ../../Info.plist ./
cp ../../PkgInfo ./
chmod a-w *

# Prepare Resoulces folder
mkdir Resources
cd Resources

# Copy application icon
cp ../../../AirspaceConverter.icns ./
chmod a-w ./AirspaceConverter.icns

# Copy KML icons
cp -r ../../../../icons ./
chmod a-w ./icons/*

cd ..

# Prepare MacOS directory (for binaries)
mkdir MacOS
cd MacOS

# Copy AirspaceConverter dynamic shared library
cp ../../../../Release/libairspaceconverter.dylib ./

# Copy AirspaceConverter CLI
cp ../../../../Release/airspaceconverter ./

# Copy AirspaceConverter Qt GUI exectutable
cp ../../../../buildQt/airspaceconverter-gui.app/Contents/MacOS/airspaceconverter-gui ./

# To find out the required libraries:
#$ otool -L airspaceconverter

# Copy all the other the required libaries into the bundle
cp /usr/local/opt/libzip/lib/libzip.5.dylib ./
cp /usr/local/opt/boost/lib/libboost_filesystem.dylib ./
cp /usr/local/opt/boost/lib/libboost_system.dylib ./
cp /usr/local/opt/boost/lib/libboost_system-mt.dylib ./
cp /usr/local/opt/boost/lib/libboost_locale-mt.dylib ./
cp /usr/local/opt/boost/lib/libboost_chrono-mt.dylib ./
cp /usr/local/opt/boost/lib/libboost_thread-mt.dylib ./
cp /usr/local/opt/icu4c/lib/libicudata.64.dylib ./
cp /usr/local/opt/icu4c/lib/libicui18n.64.dylib ./
cp /usr/local/opt/icu4c/lib/libicuuc.64.dylib ./
cp /Users/arealis/Qt/5.12.5/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets ./
cp /Users/arealis/Qt/5.12.5/clang_64/lib/QtGui.framework/Versions/5/QtGui ./
cp /Users/arealis/Qt/5.12.5/clang_64/lib/QtCore.framework/Versions/5/QtCore ./

# Tell the CLI executable to use the libraries in the same folder
install_name_tool -change libairspaceconverter.dylib @executable_path/libairspaceconverter.dylib ./airspaceconverter
install_name_tool -change /usr/local/opt/libzip/lib/libzip.5.dylib @executable_path/libzip.5.dylib ./airspaceconverter
install_name_tool -change /usr/local/opt/boost/lib/libboost_system.dylib @executable_path/libboost_system.dylib ./airspaceconverter
install_name_tool -change /usr/local/opt/boost/lib/libboost_filesystem.dylib @executable_path/libboost_filesystem.dylib ./airspaceconverter
install_name_tool -change /usr/local/opt/boost/lib/libboost_locale-mt.dylib @executable_path/libboost_locale-mt.dylib ./airspaceconverter

# Tell the GUI executable to use the libraries in the same folder
install_name_tool -change libairspaceconverter.dylib @executable_path/libairspaceconverter.dylib ./airspaceconverter-gui
install_name_tool -change /usr/local/opt/libzip/lib/libzip.5.dylib @executable_path/libzip.5.dylib ./airspaceconverter-gui
install_name_tool -change /usr/local/opt/boost/lib/libboost_system.dylib @executable_path/libboost_system.dylib ./airspaceconverter-gui
install_name_tool -change /usr/local/opt/boost/lib/libboost_filesystem.dylib @executable_path/libboost_filesystem.dylib ./airspaceconverter-gui
install_name_tool -change /usr/local/opt/boost/lib/libboost_locale-mt.dylib @executable_path/libboost_locale-mt.dylib ./airspaceconverter-gui
install_name_tool -change @rpath/QtWidgets.framework/Versions/5/QtWidgets @executable_path/QtWidgets ./airspaceconverter-gui
install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @executable_path/QtGui ./airspaceconverter-gui
install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/QtCore ./airspaceconverter-gui

# Tell libairspaceconverter.dylib to use the libraries in the same folder
install_name_tool -change /usr/local/opt/libzip/lib/libzip.5.dylib @loader_path/libzip.5.dylib ./libairspaceconverter.dylib
install_name_tool -change /usr/local/opt/boost/lib/libboost_filesystem.dylib @loader_path/libboost_filesystem.dylib ./libairspaceconverter.dylib
install_name_tool -change /usr/local/opt/boost/lib/libboost_system.dylib @loader_path/libboost_system.dylib ./libairspaceconverter.dylib
install_name_tool -change /usr/local/opt/boost/lib/libboost_locale-mt.dylib @loader_path/libboost_locale-mt.dylib ./libairspaceconverter.dylib

# Tell Boost locale library to use the libraries in the same folder
chmod u+w libboost_locale-mt.dylib
install_name_tool -change /usr/local/opt/icu4c/lib/libicudata.64.dylib @loader_path/libicudata.64.dylib ./libboost_locale-mt.dylib
install_name_tool -change /usr/local/opt/icu4c/lib/libicui18n.64.dylib @loader_path/libicui18n.64.dylib ./libboost_locale-mt.dylib
install_name_tool -change /usr/local/opt/icu4c/lib/libicuuc.64.dylib @loader_path/libicuuc.64.dylib ./libboost_locale-mt.dylib

# And also for the Qt libs
install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @loader_path/QtCore ./QtGui
install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @loader_path/QtCore ./QtWidgets
install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @loader_path/QtGui ./QtWidgets


# Set executables and libraries permissions
chmod a-w *
chmod a+x *

# Get out
cd ../../..

echo "Make macOS application bundle done."
