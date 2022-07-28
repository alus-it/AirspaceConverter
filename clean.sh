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

# Clean everything
echo Clean everything...

# Clean Qt user interface
rm -R -f buildQt

# Clean shared library and command line version
make clean

# Clean macOS stuff
cd macOS
./clean.sh
cd ..

echo "Full clean done."

exit 0

