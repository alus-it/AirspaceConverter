#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 07/09/2020
# Author      : Valerio Messina <efa@iol.it>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2021 Valerio Messina
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================
# 'tstCsv' check the CSV in/out comparing to a reference generated with CUP2CSV bash script

cd ..
echo Compiling ...
build.sh
if (test "$?" != 0) then
   echo compile error
   exit
fi
echo Done
cd Release
echo ""

echo Testing CUP2CSV ...
#airspaceconverter -w ~/Documents/Flight/AIP/tst.cup -o ~/Documents/Flight/AIP/tst.csv
airspaceconverter -w ~/Documents/Flight/AIP/ITA_WPT_10-SEP-2020-2010_V03.cup -o ~/Documents/Flight/AIP/tst.csv
echo -en "\e[1A" # cursor up 1 line
echo Sorting ...
sort ~/Documents/Flight/AIP/tst.csv > ~/Documents/Flight/AIP/tst1.csv
mv ~/Documents/Flight/AIP/tst1.csv ~/Documents/Flight/AIP/tst.csv
echo Comparing ...
#diff --strip-trailing-cr ~/Documents/Flight/AIP/ref.csv ~/Documents/Flight/AIP/tst.csv > ~/Documents/Flight/AIP/diff.txt
diff --strip-trailing-cr ~/Documents/Flight/AIP/ITA_WPT_10-SEP-2020-2010_V03.csv ~/Documents/Flight/AIP/tst.csv > ~/Documents/Flight/AIP/diff.txt
if (! test -s ~/Documents/Flight/AIP/diff.txt) then
   echo OK
fi
echo ""

echo Testing CSV2CUP ...
airspaceconverter -w ~/Documents/Flight/AIP/tst2.csv -o ~/Documents/Flight/AIP/tst2.cup
echo -en "\e[1A" # cursor up 1 line
echo ""

echo Testing CSV2CSV ...
airspaceconverter -w ~/Documents/Flight/AIP/tst2.csv -o ~/Documents/Flight/AIP/tst3.csv
echo -en "\e[1A" # cursor up 1 line
echo Sorting ...
sort ~/Documents/Flight/AIP/tst3.csv > ~/Documents/Flight/AIP/tst4.csv
mv ~/Documents/Flight/AIP/tst4.csv ~/Documents/Flight/AIP/tst3.csv
