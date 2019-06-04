#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 9/12/2017
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : http://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2019 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Compile everything

echo Building everything...

# Get number of processors available
PROCESSORS="$(grep -c ^processor /proc/cpuinfo)"

# Build shared library and command line version
make -j${PROCESSORS} all
if [ "$?" -ne 0 ]; then
	echo "ERROR: Failed to compile shared library and CLI executable."
	exit 1
fi

# Build Qt user interface
mkdir -p buildQt
cd buildQt
qmake ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec linux-g++-64
make -j${PROCESSORS} all

if [ "$?" -ne 0 ]; then
	echo "ERROR: Failed to compile Qt user interface."
	cd ..
	exit 1
fi

strip -S --strip-unneeded ./airspaceconverter-gui

cd ..

echo Done.

exit 0

