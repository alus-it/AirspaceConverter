#!/bin/bash

# Ask version number
echo Version number?
read version

# Get number of processors available
PROCESSORS="$(grep -c ^processor /proc/cpuinfo)"

# Build shared library and command line version
make -j${PROCESSORS} all

# Build Qt user interface
mkdir QtBuild
cd QtBuild
qmake ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec linux-g++-64
make -j${PROCESSORS} all
cd ..

# Man page has to be compressed at maximum
gzip -9 < airspaceconverter.1 > airspaceconverter.1.gz

# Make folders for packages
#TODO....

# Copy all the files
#TODO...

#Root has to be the owner of all files
sudo chown -R root:root *

#Make DEB package
dpkg-deb --build airspaceconverter_${version}-1_amd64

# Clean
# TODO rm directories
rm -R QtBuild

