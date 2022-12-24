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

# First compile
./build.sh

# Abort if build failed	
if [ "$?" -ne 0 ]; then
	exit 1
fi

if [ "$(uname)" == "Darwin" ]; then
	echo "Installing everything on macOS..."
	
	# Install shared library and CLI
	make install
	
	# Install GUI
	cd macOS
	./install.sh
	cd ..
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	echo "Installing everything on Linux ..."
	
	# Install shared library and CLI
	sudo make install
	
	# Install GUI
	sudo cp ./Release/airspaceconverter-gui /usr/bin
	sudo chmod 0755 /usr/bin/airspaceconverter-gui
else
	echo "ERROR: this script is only for Linux or macOS ..."
	exit 1
fi

echo "Full installation done."