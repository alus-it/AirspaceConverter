#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 08/12/2017
# Author      : Valerio Messina <efa@iol.it>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2023 Valerio Messina
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================
# 'test' build, copy binaries and test Airspaces files in test dir, start GUI

cd ..
echo Compiling ...
build.sh
if (test "$?" != 0) then
   echo compile error
   exit
fi
echo Done
echo Copying binaries ...
cp Release/airspaceconverter test
cp Release/libairspaceconverter.so test
cp buildQt/airspaceconverter-gui test
cd test
echo Done
echo Copying aip airspaces files ...
cp ~/Documents/Flight/airspaces/aip_in/openaip_airspace_ts.aip .
cp ~/Documents/Flight/airspaces/aip_in/Amendola.aip .
cp ~/Documents/Flight/airspaces/aip_in/openaip_airspace_italy_it.aip .
cp ~/Documents/Flight/airspaces/aip_in/openaip_airspace_switzerland_ch.aip .
echo Done
airspaceconverter-gui &
