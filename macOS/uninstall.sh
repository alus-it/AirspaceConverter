#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 18/2/2020
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2021 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Remove older version of GUI application if present
if [[ -d /Applications/AirspaceConverter.app ]]
then
	echo "Removing already installed AirspaceConverter GUI application"
	rm -rf /Applications/AirspaceConverter.app/
fi

echo "MacOS GUI application uninstallation done."