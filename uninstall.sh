#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 9/12/2017
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2019 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

echo Uninstalling everything...

# Uninstall GUI
sudo rm -f /usr/bin/airspaceconverter-gui

# Uninstall shared library and CLI
sudo make uninstall

echo Done.

