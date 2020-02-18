#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 17/2/2020
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

# Build macOS application bundle if not yet done
if [[ ! -d ./AirspaceConverter.app ]]
then
	echo "Building macOS application bundle which was not yed done..."
	./makeApp.sh
fi

# Launch the AirspaceConverter application from here (not the installed one)
echo "Launching local AirspaceConverter application"
open ./AirspaceConverter.app
