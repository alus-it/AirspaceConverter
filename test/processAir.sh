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

airspaceconverter -i openaip_airspace_ts.aip -o openair_airspace_ts.txt -p -s

airspaceconverter -i openaip_airspace_sixty.aip -o openair_airspace_sixty.txt -p -s

airspaceconverter -i Amendola.aip -o Amendola.txt -p -s

airspaceconverter -i openaip_airspace_switzerland_ch.aip -o openair_airspace_ch.txt -p -s

airspaceconverter -i openaip_airspace_italy_it.aip -o openair_airspace_it.txt -p -s

airspaceconverter -i openaip_airspace_italy_it.aip -i openaip_airspace_switzerland_ch.aip -l 90,44.8,-180,180 -o openair_airspace_cli.txt -p -s
