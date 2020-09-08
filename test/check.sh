#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 08/12/2017
# Author      : Valerio Messina <efa@iol.it>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : Copyright 2016-2020 Valerio Messina
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================
# 'check' sort Openair test files and compare. Depend on 'openairSort'

fileIn=openair_airspace_ts.txt
fileIn=openair_airspace_sixty.txt
fileIn=Amendola.txt
fileIn=openair_airspace_ch.txt
fileIn=openair_airspace_it.txt
fileIn=openair_airspace_cli.txt
fileOut=openair_airspace_check.txt

comp=opentxt_airspace_switzerland_ch.txt
comp=opentxt_airspace_italy_it.txt
comp=opentxt_airspace_lat.txt

cp $fileIn t0.txt
dos2unix -q t0.txt
cat t0.txt | sed 's/\([[:digit:]]\) FT /\1F /g' > t1.txt
cat t1.txt | sed 's/ AMSL$/ MSL$/g' > t2.txt
cat t2.txt | sed 's/F AGL$/F GND$/g' > t3.txt
perl -0777 -pe 's/\n(AL .*)\n(AH .*)\n/\n\2\n\1\n/g' t3.txt > t4.txt
#unix2dos -q t4.txt
cp t4.txt $fileOut
rm t0.txt t1.txt t2.txt t3.txt t4.txt
openairSort openair_airspace_check.txt openair_0.2.6_sorted.txt

cp ~/Documents/Flight/airspaces/openair_out/$comp .
cat $comp | sed 's/^AC X$/AC G/g' > t0.txt
mv t0.txt $comp
openairSort $comp opentxt_php_sorted.txt

diff opentxt_php_sorted.txt openair_0.2.6_sorted.txt > diff.txt
ne opentxt_php_sorted.txt openair_0.2.6_sorted.txt diff.txt
