#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 07/09/2020
# Author      : Valerio Messina <efa@iol.it>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : Copyright 2016-2020 Valerio Messina
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================
# 'tstCsv' check the CSV output comparing to a reference generated with CUP2CSV bash script

cd ..
echo Compiling ...
build.sh
if (test "$?" != 0) then
   echo compile error
   exit
fi
echo Done
cd Release
#airspaceconverter -w ~/Documents/Flight/AIP/ITA_WPT_30-JAN-2020-2002_V02.cup -o ~/Documents/Flight/AIP/tst.csv
airspaceconverter -w ~/Documents/Flight/AIP/tst.cup -o ~/Documents/Flight/AIP/tst.csv
echo Sorting ...
sort ~/Documents/Flight/AIP/tst.csv > ~/Documents/Flight/AIP/tst2.csv
mv ~/Documents/Flight/AIP/tst2.csv ~/Documents/Flight/AIP/tst.csv
echo Comparing ...
#diff ~/Documents/Flight/AIP/ITA_WPT_30-JAN-2020-2002_V02.csv ~/Documents/Flight/AIP/tst.csv
diff ~/Documents/Flight/AIP/ref.csv ~/Documents/Flight/AIP/tst.csv
