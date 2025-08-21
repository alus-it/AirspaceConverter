#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 10/9/2017
# Authors     : Alberto Realis-Luc <alberto.realisluc@gmail.com>
#               Valerio Messina <efa@iol.it>
# Web         : https://www.alus.it/AirspaceConverter
# Copyright   : (C) 2017 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Make sure that we are on Linux
if [[ "$(uname)" == "Darwin" || "$(expr substr $(uname -s) 1 5)" != "Linux" ]]; then
	echo "ERROR: this script is only for Linux ..."
	exit 1
fi

# Determine if we want to compile or just copy binaries
printf "Compile [C] on this machine or only build the DEB package [D] from existing binaries [C/D]? "
read -r ACTION

if [[ "$ACTION" == "C" || "$ACTION" == "c" || "$ACTION" == "" ]]; then
	# Find out Debian version
	if [ -f /etc/os-release ]; then
		# freedesktop.org and systemd
		. /etc/os-release
		OS=$NAME
		OSVER=$VERSION_ID
		CODENAME=$DEBIAN_CODENAME
	elif type lsb_release >/dev/null 2>&1; then
		# linuxbase.org
		OS=$(lsb_release -si)
		OSVER=$(lsb_release -sr)
	elif [ -f /etc/lsb-release ]; then
		# For some versions of Debian/Ubuntu without lsb_release command
		. /etc/lsb-release
		OS=$DISTRIB_ID
		OSVER=$DISTRIB_RELEASE
	elif [ -f /etc/debian_version ]; then
		# Older Debian/Ubuntu/etc.
		OS=Debian
		OSVER=$(cat /etc/debian_version)
	#elif [ -f /etc/SuSe-release ]; then
		# Older SuSE/etc.
	#elif [ -f /etc/redhat-release ]; then
		# Older Red Hat, CentOS, etc.
	else
		# Fall back to uname, e.g. "Linux <version>", also works for BSD, etc.
		OS=$(uname -s)
		OSVER=$(uname -r)
	fi

	# Check if it is Debian, Ubuntu or MintDebianEdition
	if [ "$OS" != "Debian GNU/Linux" ] && [ "$OS" != "Ubuntu" ] && [ "$OS" != "LMDE" ]; then
		echo "ERROR: this script can actually work only for Debian, Ubuntu or LMDE."
		echo "You may consider to update it!"
		exit 1
	fi

	# Get architecture type
	case $(uname -m) in
		x86_64)
			ARCH=amd64
			;;
		i686)
			ARCH=i386
			;;
		*)
			echo "ERROR: This script is not able to work on the achitecture, please add it!"
			exit 1
		esac

elif [[ "$ACTION" == "D" || "$ACTION" == "d" ]]; then
	# Tell the user to copy the binaries built on another Debian	
	echo "So, please copy the already compiled: airspaceconverter libairspaceconverter.so and airspaceconverter-gui binaries to this folder."
	read -p "Press ENTER when done"

	# Instead of building just copy the binaries (make sure they are executable as well)
	chmod a+x airspaceconverter
	chmod a+x airspaceconverter-gui
	mkdir -p Release
	mv airspaceconverter ./Release/airspaceconverter
	mv libairspaceconverter.so ./Release/libairspaceconverter.so
	mv airspaceconverter-gui ./Release/airspaceconverter-gui

	# Ask the user for which version of Debian we are building the packages	
	printf "Enter target Debian [7,8,9,10] or Ubuntu [16.04,18.04,20.04] release number: "
	read -r OSVER

	# Ask the packager for which architecure are built the copied binaries
	printf "How many bits has the target architecture? [32/64]: "
	read -r ARCH
	if [[ "$ARCH" == "64" || "$ARCH" == "" ]]; then
		ARCH=amd64
	elif [ "$ARCH" == "32" ]; then
		ARCH=i386
	else
		echo "ERROR: Unknown achitecture, please add it!"
		exit 1
	fi
else
	echo "ERROR: Unknown action!"
	exit 1
fi

# Check which Debian or Ubuntu release we are talking about
case $OSVER in
	7)
		echo "Packaging for Debian 7 Wheezy..."
		DISTR=deb
		LIBCVER=2.13
		ZIPLIB=libzip2
		ZIPVER=0.10.1
		BOOSTVER=1.49.0
		QTVER=4:4.8.2
		QTDEPS="libqtcore4 (>= ${QTVER}), libqtgui4 (>= ${QTVER})"
		MANT="Alberto Realis-Luc <admin@alus.it>"
		;;
	8)
		echo "Packaging for Debian 8 Jessie..."
		DISTR=deb
		LIBCVER=2.19
		ZIPLIB=libzip2
		ZIPVER=0.11.2
		BOOSTVER=1.55.0
		QTVER=5.3.2
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 10.3.2)"
		MANT="Alberto Realis-Luc <admin@alus.it>"
		;;
	9)
		echo "Packaging for Debian 9 Stretch..."
		DISTR=deb
		LIBCVER=2.19
		ZIPLIB=libzip4
		ZIPVER=1.1.2
		BOOSTVER=1.62.0
		QTVER=5.7.1
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 10.3.2)"
		MANT="Alberto Realis-Luc <admin@alus.it>"
		;;
	10)
		echo "Packaging for Debian 10 Buster..."
		DISTR=deb
		LIBCVER=2.28
		ZIPLIB=libzip4
		ZIPVER=1.5.1
		BOOSTVER=1.67.0
		QTVER=5.11.3
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 18.3.6)"
		MANT="Valerio Messina <efa@iol.it>"
		;;
	11)
		echo "Packaging for Debian 11 Bullseye..."
		DISTR=deb
		LIBCVER=2.31
		ZIPLIB=libzip4
		ZIPVER=1.7.3
		BOOSTVER=1.74.0
		QTVER=5.15.2
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 18.3.6)"
		MANT="Valerio Messina <efa@iol.it>"
		;;
	12)
		echo "Packaging for Debian 12 Bookworm..."
		DISTR=deb
		LIBCVER=2.36
		ZIPLIB=libzip4
		ZIPVER=1.7.3
		BOOSTVER=1.74.0
		QTVER=5.15.8
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 18.3.6)"
		MANT="Valerio Messina <efa@iol.it>"
		;;
	13)
		echo "Packaging for Debian 13 Trixie..."
		DISTR=deb
		LIBCVER=2.41
		ZIPLIB=libzip4
		ZIPVER=1.7.3
		BOOSTVER=1.83.0
		QTVER=5.15.15
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 18.3.6)"
		MANT="Valerio Messina <efa@iol.it>"
		;;
	4)
		echo "Packaging for LMDE4 (Debian 10 Buster)..."
		DISTR=lmde
		LIBCVER=2.28
		ZIPLIB=libzip4
		ZIPVER=1.5.1
		BOOSTVER=1.67.0
		QTVER=5.11.3
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 18.3.6)"
		MANT="Valerio Messina <efa@iol.it>"
		;;
	5)
		echo "Packaging for LMDE5 (Debian 11 Bullseye)..."
		DISTR=lmde
		LIBCVER=2.31
		ZIPLIB=libzip4
		ZIPVER=1.7.3
		BOOSTVER=1.74.0
		QTVER=5.15.2
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 18.3.6)"
		MANT="Valerio Messina <efa@iol.it>"
		;;
	16.04)
		echo "Packaging for Ubuntu 16.04 Xenial..."
		DISTR=ubn
		LIBCVER=2.23
		ZIPLIB=libzip4
		ZIPVER=1.0.1
		BOOSTVER=1.58.0
		QTVER=5.5.1
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 10.3.2)"
		MANT="Valerio Messina <efa@iol.it>"
		;;
	18.04)
		echo "Packaging for Ubuntu 18.04 Bionic..."
		DISTR=ubn
		LIBCVER=2.27
		ZIPLIB=libzip4
		ZIPVER=1.1.2
		BOOSTVER=1.65.1
		QTVER=5.9.5
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 10.3.2)"
		MANT="Valerio Messina <efa@iol.it>"
		;;
	20.04)
		echo "Packaging for Ubuntu 20.04 Focal..."
		DISTR=ubn
		LIBCVER=2.31
		ZIPLIB=libzip5
		ZIPVER=1.5.1
		BOOSTVER=1.71.0
		QTVER=5.12.8
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER})"
		MANT="Alberto Realis-Luc <admin@alus.it>"
		;;
	22.04)
		echo "Packaging for Ubuntu 22.04 Jammy..."
		DISTR=ubn
		LIBCVER=2.35
		ZIPLIB=libzip5
		ZIPVER=1.5.1
		BOOSTVER=1.74.0
		QTVER=5.15.3
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER})"
		MANT="Alberto Realis-Luc <admin@alus.it>"
		;;
	24.04)
		echo "Packaging for Ubuntu 24.04 Noble..."
		DISTR=ubn
		LIBCVER=2.39
		ZIPLIB=libzip4t64
		ZIPVER=1.7.3
		BOOSTVER=1.83.0
		QTVER=5.15.13
		QTDEPS="libqt5core5t64 (>= ${QTVER}), libqt5gui5t64 (>= ${QTVER}), libqt5widgets5t64 (>= ${QTVER})"
		MANT="Alberto Realis-Luc <admin@alus.it>"
		;;
	*)
	echo "ERROR: This version of Debian, Ubuntu or Mint: ${OSVER} is not known by this script, please add it!"
	exit 1
esac

if [[ "$ACTION" == "C" || "$ACTION" == "c" || "$ACTION" == "" ]]; then
	
	# Compile everything
	./build.sh

	# Abort if compile failed	
	if [ "$?" -ne 0 ]; then
		exit 1
	fi
fi

# Get version number, try from the sources (sources should be present)
VERSION="$(grep -s "define VERSION" src/AirspaceConverter.hpp | awk -F\" '{print $2}')"

# Other possibility: get version number from compiled executable (not needed it should be the same as from the sources)
#cd Release # so find local shared object: libairspaceconverter.so #WARNING: not always true, can be also the installed lib!!
#VERSION="$(./airspaceconverter -v | head -n 1 | sed -r 's/^[^0-9]+([0-9]+.[0-9]+.[0-9]+).*/\1/g')"
#cd ..

# If no valid version number, then ask the user
if [[ "$VERSION" =~ [0-9].[0-9].[0-9] ]]; then
	echo New airspaceconverter version: $VERSION
else
	printf "Enter new airspaceconverter version: "
	read -r VERSION
fi

# Man page has to be compressed at maximum and without timestamp
gzip -9 -n < airspaceconverter.1 > airspaceconverter.1.gz

# Make folder for airspaceconverter
sudo rm -rf airspaceconverter_${VERSION}-${DISTR}${OSVER}_${ARCH}
mkdir airspaceconverter_${VERSION}-${DISTR}${OSVER}_${ARCH}
cd airspaceconverter_${VERSION}-${DISTR}${OSVER}_${ARCH}

# Build directory structure
mkdir usr
chmod 0755 usr
cd usr
mkdir bin
mkdir lib
mkdir share
chmod 0755 bin lib share
cd share
mkdir airspaceconverter
chmod 0755 airspaceconverter
cd airspaceconverter
mkdir icons
chmod 0755 icons
cd ..
mkdir doc
chmod 0755 doc
cd doc
mkdir airspaceconverter
chmod 0755 airspaceconverter
cd airspaceconverter

# Make copyright file
echo 'Format-Specification: http://svn.debian.org/wsvn/dep/web/deps/dep5.mdwn?op=file&rev=135
Name: airspaceconverter
Maintainer: Alberto Realis-Luc <admin@alus.it>
Source: https://github.com/alus-it/AirspaceConverter.git

Copyright: (C) 2016 Alberto Realis-Luc <alberto.realisluc@gmail.com>
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
chmod 0644 copyright

cd ..
cd ..
mkdir man
chmod 0755 man
cd man
mkdir man1
chmod 0755 man1
cd ..
cd ..

# Copy the files
cp ../../Release/airspaceconverter ./bin
chmod 0755 ./bin/airspaceconverter
cp ../../Release/libairspaceconverter.so ./lib
chmod 0644 ./lib/libairspaceconverter.so
cp ../../icons/* ./share/airspaceconverter/icons
chmod 0644 ./share/airspaceconverter/icons/*.png
mv ../../airspaceconverter.1.gz ./share/man/man1
chmod 0644 ./share/man/man1/airspaceconverter.1.gz

cd ..

# Estimate package size
SIZE="$(du -s --apparent-size usr | awk '{print $1}')"
#echo SIZE:$SIZE

# Make control directory
mkdir DEBIAN
cd DEBIAN

# Make control file
echo 'Package: airspaceconverter
Version: '${VERSION}'-'${DISTR}${OSVER}'
Section: misc
Priority: optional
Build-Depends: libc6-dev (>= '${LIBCVER}'), libboost-system-dev (>= '${BOOSTVER}'), libboost-filesystem-dev (>= '${BOOSTVER}'), libboost-locale-dev (>= '${BOOSTVER}'), libzip-dev (>= '${ZIPVER}')
Standards-Version: 3.9.4
Architecture: '${ARCH}'
Depends: libc6 (>= '${LIBCVER}'), libboost-system'${BOOSTVER}' (>= '${BOOSTVER}'), libboost-filesystem'${BOOSTVER}' (>= '${BOOSTVER}'), libboost-locale'${BOOSTVER}' (>= '${BOOSTVER}'), '${ZIPLIB}' (>= '${ZIPVER}')
Enhances: airspaceconverter-gui (>= '${VERSION}'), cgpsmapper (>= 0.0.9.3c)
Maintainer: '${MANT}'
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
 portable "core", it works from "command line" but a graphic interface (package
 airspaceconverter-gui) is also available.
Homepage: https://www.alus.it/AirspaceConverter/
' > control

cd ..

#Root has to be the owner of all files
sudo chown -R root:root *
cd ..

#Make airspaceconverter DEB package
rm -f airspaceconverter_${VERSION}-${DISTR}${OSVER}_${ARCH}.deb
dpkg-deb --build airspaceconverter_${VERSION}-${DISTR}${OSVER}_${ARCH}

# Make folder for airspaceconverter-gui
sudo rm -rf airspaceconverter-gui_${VERSION}-${DISTR}${OSVER}_${ARCH}
mkdir airspaceconverter-gui_${VERSION}-${DISTR}${OSVER}_${ARCH}
cd airspaceconverter-gui_${VERSION}-${DISTR}${OSVER}_${ARCH}

# Build directory structure
mkdir usr
chmod 0755 usr
cd usr
mkdir bin
mkdir share
chmod 0755 bin share
cd share
mkdir applications
chmod 0755 applications
cd applications
echo '[Desktop Entry]
Version=1.0
Type=Application
Name=AirspaceConverter
Comment=
Exec=airspaceconverter-gui
Terminal=false
StartupNotify=no
Icon=airspaceconverter.xpm
Categories=Education;Science;Geoscience' > airspaceconverter-gui.desktop
chmod 0644 airspaceconverter-gui.desktop
cd ..
mkdir doc
chmod 0755 doc
cd doc
mkdir airspaceconverter-gui
chmod 0755 airspaceconverter-gui
cd ..
mkdir menu
chmod 0755 menu
cd menu
echo '?package(airspaceconverter-gui): needs="X11"\
   section="Applications/Science/Geoscience"\
   title="Airspace Converter"\
   icon="/usr/share/pixmaps/airspaceconverter.xpm"\
   command="/usr/bin/airspaceconverter-gui"' > airspaceconverter-gui
chmod 0644 airspaceconverter-gui
cd ..
mkdir pixmaps
chmod 0755 pixmaps
cd ..

# Copy the files
cp ../../airspaceconverter_${VERSION}-${DISTR}${OSVER}_${ARCH}/usr/share/doc/airspaceconverter/copyright ./share/doc/airspaceconverter-gui
cp ../../airspaceconverter.xpm ./share/pixmaps
chmod 0644 ./share/pixmaps/airspaceconverter.xpm
cp ../../Release/airspaceconverter-gui ./bin
chmod 0755 ./bin/airspaceconverter-gui

cd ..

# Estimate package size
SIZE="$(du -s --apparent-size usr | awk '{print $1}')"
#echo SIZE:$SIZE
# Make control directory
mkdir DEBIAN
cd DEBIAN

# Make control file
echo 'Package: airspaceconverter-gui
Version: '${VERSION}'-'${DISTR}${OSVER}'
Section: misc
Priority: optional
Standards-Version: 3.9.4
Architecture: '${ARCH}'
Depends: libc6 (>= '${LIBCVER}'), airspaceconverter (>= '${VERSION}'), '${QTDEPS}', libboost-system'${BOOSTVER}' (>= '${BOOSTVER}') ,libboost-filesystem'${BOOSTVER}' (>= '${BOOSTVER}')
Suggests: airspaceconverter (>= '${VERSION}')
Enhances: cgpsmapper (>= 0.0.9.3c)
Maintainer: '${MANT}'
Installed-size: '${SIZE}'
Description: Qt GUI for Airspace Converter
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
 contain the size of output files. This package is the graphical interface of
 AirsapceConverter. You can install only the package airspaceconverter if you
 wish to use it purely from command line.
Homepage: https://www.alus.it/AirspaceConverter/
' > control
#chmod 0755 control

# Make post inst script
echo '#!/bin/sh
set -e
if [ "$1" = "configure" ] && [ -x "`which update-menus 2>/dev/null`" ]; then
	update-menus
fi
' > postinst
chmod 0755 postinst

# Make post removal script
echo '#!/bin/sh
set -e
if [ -x "`which update-menus 2>/dev/null`" ]; then update-menus ; fi
' > postrm
chmod 0755 postrm

cd ..

#Root has to be the owner of all files
sudo chown -R root:root *
cd ..

#Make DEB package for airspaceconverter-gui
rm -f airspaceconverter-gui_${VERSION}-${DISTR}${OSVER}_${ARCH}.deb
dpkg-deb --build airspaceconverter-gui_${VERSION}-${DISTR}${OSVER}_${ARCH}

# Clean
echo Cleaning temp files...
sudo rm -R airspaceconverter_${VERSION}-${DISTR}${OSVER}_${ARCH}
sudo rm -R airspaceconverter-gui_${VERSION}-${DISTR}${OSVER}_${ARCH}
printf "Clean build directory [Y/N]: "
read -r CLEAN
if [[ "$CLEAN" == "Y" || "$CLEAN" == "y" ]]; then
	echo Cleaning...
	./clean.sh
fi

exit 0
