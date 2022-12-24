#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 9/12/2017
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2023 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================
# On Linux: to run this script it is necessary to have lsb-release installed

# First find out where we want to build and number of processors available
QMAKE=qmake
if [ "$(uname)" == "Darwin" ]; then
	SYSTEM="macOS"
	PROCESSORS=2
	echo "Building everything for macOS ..."
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	SYSTEM="Linux"
	PROCESSORS="$(grep -c ^processor /proc/cpuinfo)"
	echo "Building everything for Linux ..."
	if (test `lsb_release -si` = "Fedora") then
		QMAKE=qmake-qt4
	fi
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
	$QMAKE ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec linux-g++-64
else
	$QMAKE ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec macx-clang CONFIG+=x86_64
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
	mv ./airspaceconverter-gui ../Release/airspaceconverter-gui
else
	strip -S ./airspaceconverter-gui.app/Contents/MacOS/airspaceconverter-gui
fi
cd ..

echo "Full build done."
exit 0
