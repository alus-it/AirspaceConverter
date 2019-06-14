#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 8/12/2017
# Author      : Valerio Messina <efa@iol.it>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2019 Valerio Messina
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# 'openairSort' sort openair airspaces, keep duplicates, v1.00
fileIn1=$1
fileOut=$2

cp $fileIn1 tmp1.txt
dos2unix -q tmp1.txt
cat tmp1.txt | awk 'BEGIN{RS="\n\n" ; ORS="[";}{ print }' > tmp2.txt # 2xNewLine to [
cat tmp2.txt | tr '\n' ']' > tmp1.txt # 1xNewLine to ]
cat tmp1.txt | tr '[' '\n' > tmp2.txt # [ to 1xNewLine
cp tmp2.txt out.txt

airIn=`wc -l out.txt | cut -d' ' -f1`
echo airIn:$airIn airspaces
cat out.txt | sort > tmp1.txt # sort
airOut=`wc -l tmp1.txt | cut -d' ' -f1`
echo airOut:$airOut airspaces

cat tmp1.txt | awk 'BEGIN{RS="\n" ; ORS="\n\n";}{ print }' > tmp2.txt # 1xNewLine to 2xNewLine
cat tmp2.txt | tr ']' '\n' > tmp1.txt # ] to 1xNewLine
mv tmp1.txt $fileOut # output file
rm tmp2.txt out.txt
echo Note: Output file \"$fileOut\" has Unix newlines
