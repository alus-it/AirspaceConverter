#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 18/2/2020
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2023 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Make sure that we are on macOS
if [ "$(uname)" != "Darwin" ]; then
	echo "ERROR: this script is only for macOS ..."
	exit 1
fi

# First remove older version if present
./uninstall.sh

# Build macOS application bundle if not yet done
if [[ ! -d ./AirspaceConverter.app ]]
then
	echo "Building macOS application bundle which was not yet done..."
	./makeApp.sh
fi

# Install AirspaceConverter Qt GUI application ...
echo "Installing  AirspaceConverter GUI application ..."
cp -r ./AirspaceConverter.app /Applications
echo "MacOS application installation done."

# Launch installed application
open /Applications/AirspaceConverter.app
