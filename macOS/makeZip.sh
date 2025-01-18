#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 17/2/2020
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2020 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Make sure that we are on macOS
if [ "$(uname)" != "Darwin" ]; then
	echo "ERROR: this script is only for macOS ..."
	exit 1
fi

# First clean older files
rm -rf AirspaceConverter*.zip

# Build macOS application bundle if not yet done
if [[ ! -d ./AirspaceConverter.app ]]
then
	echo "Building macOS application bundle which was not yed done..."
	./makeApp.sh
fi

# Get version number, try from the sources (sources should be present)
VERSION="$(grep -s "define VERSION" ../src/AirspaceConverter.h | awk -F\" '{print $2}')"

# If no valid version number, then ask the user
if [[ "$VERSION" =~ [0-9].[0-9].[0-9] ]]; then
	echo "Detected AirspaceConverter version: ${VERSION}"
else
	printf "Enter new airspaceconverter version: "
	read -r VERSION
fi

# Remove the dots from the version string
IFS='.'
arr=($VERSION)
VER="${arr[0]]}${arr[1]]}${arr[2]]}"

# Create distribution ZIP file
echo "Building distribution ZIP file ..."
zip -9 -T -r AirspaceConverter${VER}.zip AirspaceConverter.app/

echo "Distribution ZIP file: AirspaceConverter${VER}.zip done."
