#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 9/12/2017
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2022 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

if [ "$(uname)" == "Darwin" ]; then
	echo "Uninstalling everything from macOS ..."
	
	# Uninstall GUI
	cd macOS
	./uninstall.sh
	cd ..

	# Uninstall shared library and CLI
	make uninstall
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	echo "Uninstalling everything from Linux ..."
	
	# Uninstall GUI
	sudo rm -f /usr/bin/airspaceconverter-gui

	# Uninstall shared library and CLI
	sudo make uninstall
else
	echo "ERROR: this script is only for Linux or macOS ..."
	exit 1
fi

echo "AirspaceConverter full uninstallation done."