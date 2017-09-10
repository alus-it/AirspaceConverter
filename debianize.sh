#!/bin/bash

# Ask version number
echo Version number?
read version

# Get number of processors available
PROCESSORS="$(grep -c ^processor /proc/cpuinfo)"

# Build shared library and command line version
make -j${PROCESSORS} all

# Build Qt user interface
mkdir buildQt
cd buildQt
qmake ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec linux-g++-64
make -j${PROCESSORS} all
cd ..

# Man page has to be compressed at maximum
gzip -9 < airspaceconverter.1 > airspaceconverter.1.gz

# Make folder for airspaceconverter
sudo rm -rf airspaceconverter_${version}-1_amd64
mkdir airspaceconverter_${version}-1_amd64
cd airspaceconverter_${version}-1_amd64

# Build directory structure
mkdir usr
cd usr
mkdir bin
mkdir lib
mkdir share
cd share
mkdir airspaceconverter
cd airspaceconverter
mkdir icons
cd ..
mkdir doc
cd doc
mkdir airspaceconverter
cd airspaceconverter

# Make copyright file
echo 'Format-Specification: http://svn.debian.org/wsvn/dep/web/deps/dep5.mdwn?op=file&rev=135
Name: airspaceconverter
Maintainer: Alberto Realis-Luc <admin@alus.it>
Source: https://github.com/alus-it/AirspaceConverter.git

Copyright: 2016-2017 Alberto Realis-Luc <alberto.realisluc@gmail.com>
License: GPL-3+

Files: icons/*
Copyright: 2016 Maps Icons Collection https://mapicons.mapsmarker.com
License: Creative Commons 3.0 BY-SA

License: GPL-3+
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version. 
 .
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 .
 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 .
 On Debian systems, the full text of the GNU General Public
 License version 3 can be found in the file
 `/usr/share/common-licenses/GPL-3''.' > copyright

cd ..
cd ..
mkdir man
cd man
mkdir man1
cd ..
cd ..

# Copy the files
cp ../../Release/airspaceconverter ./bin/airspaceconverter
cp ../../Release/libairspaceconverter.so ./lib/libairspaceconverter.so
cp ../../icons/* ./share/airspaceconverter/icons
mv ../../airspaceconverter.1.gz ./share/man/man1/

cd ..

# Estimate package size
SIZE="$(du -s --apparent-size usr | awk '{print $1}')"

# Make control directory
mkdir DEBIAN
cd DEBIAN

# Make control file
echo 'Package: airspaceconverter
Version: '${version}'-1
Section: misc
Priority: optional
Build-Depends: libc6-dev (>= 2.19), libboost-system-dev (>= 1.55.0), libboost-filesystem-dev (>= 1.55.0), libboost-locale-dev (>= 1.55.0), libzip-dev (>= 0.11.2)
Standards-Version: 0.2.4
Architecture: amd64
Depends: libc6 (>= 2.19), libboost-system-dev (>= 1.55.0), libboost-filesystem-dev (>= 1.55.0), libboost-locale-dev (>= 1.55.0), libzip-dev (>= 0.11.2)
Enhances: airspaceconverter-gui (>= '${version}'), cgpsmapper (>= 0.0.9.3c)
Maintainer: Alberto Realis-Luc <admin@alus.it>
Installed-size: '${SIZE}'
Description: Airspace Converter
 A free and open source tool to convert from OpenAir or OpenAIP airspace files
 to KMZ files to be shown in 3D with Google Earth.
 AirspaceConverter can take as input also SeeYou .CUP waypoints files and
 convert them as well in KMZ for Google Earth.
 This utility is also capable to output back again in OpenAir, in PFM "Polish"
 format, as file .mp, for software like: cGPSmapper or directly as file .img for
 Garmin devices obtained using directly cGPSmapper.
 The output in OpenAir is useful to make the data from OpenAIP suitable for many
 devices which support OpenAir only format; in particular this feature attempts
 to recalculate arcs and circles (possible definitions in OpenAir) in order to
 contain the size of output files. This software, written entirely in C++, has a
 portable "core", it works from "command line" on Linux.
Homepage: http://www.alus.it/AirspaceConverter/
' > control

cd ..
cd ..

#Root has to be the owner of all files
cd airspaceconverter_${version}-1_amd64
sudo chown -R root:root *
cd ..

#Make DEB package
rm -f airspaceconverter_${version}-1_amd64.deb
dpkg-deb --build airspaceconverter_${version}-1_amd64

# Clean
sudo rm -R airspaceconverter_${version}-1_amd64
rm -R buildQt

