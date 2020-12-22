#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 24/3/2020
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2021 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

echo Generating Readme.rtf...
pandoc -s ../README.md -o Readme.rtf
