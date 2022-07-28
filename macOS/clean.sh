#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 18/2/2020
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2022 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Delete application bundle and distribution files
echo "Cleaning macOS distribution files ..."
rm -rf AirspaceConverter.app
rm -f AirspaceConverter*.zip
rm -f AirspaceConverter*.dmg
