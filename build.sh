#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 9/12/2017
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2020 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Find out where whe want to build and number of processors available
if [ "$(uname)" == "Darwin" ]; then
	SYSTEM="macOS"
	PROCESSORS=2
	echo "Building everything for macOS ..."
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	SYSTEM="Linux"
	PROCESSORS="$(grep -c ^processor /proc/cpuinfo)"
	echo "Building everything for Linux ..."
else
	echo "ERROR: Unknown operating system."
	exit 1
fi

# Build shared library and command line version
echo "Building AirspaceConverter shared library and CLI executable ..."
make -j${PROCESSORS} all
if [ "$?" -ne 0 ]; then
	echo "ERROR: Failed to compile shared library and CLI executable."
	exit 1
fi

# Build Qt user interface
echo "Building AirspaceConverter Qt GUI ..."
mkdir -p buildQt
cd buildQt
if [ ${SYSTEM} == "Linux" ]; then
	qmake ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec linux-g++-64
else
	qmake ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec macx-clang CONFIG+=x86_64 CONFIG+=qtquickcompiler
fi
if [ "$?" -ne 0 ]; then
	echo "ERROR: Failed to run qmake."
	cd ..
	exit 1
fi
make -j${PROCESSORS} all
if [ "$?" -ne 0 ]; then
	echo "ERROR: Failed to compile Qt user interface."
	cd ..
	exit 1
fi
if [ ${SYSTEM} == "Linux" ]; then
	strip -S --strip-unneeded ./airspaceconverter-gui
else
	strip -S ./airspaceconverter-gui.app/Contents/MacOS/airspaceconverter-gui
fi
cd ..

echo "Done with compiling."
exit 0
