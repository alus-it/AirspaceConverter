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

echo Installing everything...

# Compile
./build.sh

# Abort if compile failed	
if [ "$?" -ne 0 ]; then
	exit 1
fi

# Install shared library and CLI
sudo make install

# Install GUI
sudo cp ./buildQt/airspaceconverter-gui /usr/bin
sudo chmod 0755 /usr/bin/airspaceconverter-gui

echo Done.

