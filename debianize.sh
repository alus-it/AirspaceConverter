#!/bin/bash
#============================================================================
# AirspaceConverter
# Since       : 10/9/2017
# Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
# Web         : http://www.alus.it/AirspaceConverter
# Copyright   : (C) 2016-2017 Alberto Realis-Luc
# License     : GNU GPL v3
#
# This script is part of AirspaceConverter project
#============================================================================

# Determine if we want to compile or just copy binaries
printf "Compile [C] on this machine or only build the DEB packge [D] from existing binaries [C/D]? "
read -r ACTION

if [[ "$ACTION" == "C" || "$ACTION" == "c" || "$ACTION" == "" ]]; then
	# Find out Debian version
	if [ -f /etc/os-release ]; then
		# freedesktop.org and systemd
		. /etc/os-release
		OS=$NAME
		DEBIANVER=$VERSION_ID
	elif type lsb_release >/dev/null 2>&1; then
		# linuxbase.org
		OS=$(lsb_release -si)
		DEBIANVER=$(lsb_release -sr)
	elif [ -f /etc/lsb-release ]; then
		# For some versions of Debian/Ubuntu without lsb_release command
		. /etc/lsb-release
		OS=$DISTRIB_ID
		DEBIANVER=$DISTRIB_RELEASE
	elif [ -f /etc/debian_version ]; then
		# Older Debian/Ubuntu/etc.
		OS=Debian
		DEBIANVER=$(cat /etc/debian_version)
	#elif [ -f /etc/SuSe-release ]; then
		# Older SuSE/etc.
	#elif [ -f /etc/redhat-release ]; then
		# Older Red Hat, CentOS, etc.
	else
	    # Fall back to uname, e.g. "Linux <version>", also works for BSD, etc.
	    OS=$(uname -s)
	    DEBIANVER=$(uname -r)
	fi

	# Check if it is Debian distribution first
	if [ "$OS" != "Debian GNU/Linux" ]; then
		echo "ERROR: this not Debian!"
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
	mkdir -p buildQt
	mv airspaceconverter-gui ./buildQt/airspaceconverter-gui
	
	# Ask the user for which version of Debian we are building the packages	
	printf "Enter target Debian release number [7-9]: "
	read -r DEBIANVER
	
	# Ask the packager for which architecure are built the copied binaries
	printf "Bits has the target architecture? [32/64]: "
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


# Check what Debian release we are talking about
case $DEBIANVER in
	7)
		echo "Packaging for Debian Wheezy..."
		LIBCVER=2.13
		ZIPLIB=libzip2		
		ZIPVER=0.10.1
		BOOSTVER=1.49.0
		QTVER=4:4.8.2
		QTDEPS="libqtcore4 (>= ${QTVER}), libqtgui4 (>= ${QTVER})"
        ;;

	8)
		echo "Packaging for Debian Jessie..."
		LIBCVER=2.19		
		ZIPLIB=libzip2		
		ZIPVER=0.11.2
		BOOSTVER=1.55.0
		QTVER=5.3.2
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 10.3.2)"
		;;

	9)
		echo "Packaging for Debian Stretch..."
		LIBCVER=2.19	
		ZIPLIB=libzip4		
		ZIPVER=1.1.2
		BOOSTVER=1.62.0
		QTVER=5.7.1
		QTDEPS="libqt5core5a (>= ${QTVER}), libqt5gui5 (>= ${QTVER}), libqt5widgets5 (>= ${QTVER}), libgl1-mesa-glx (>= 10.3.2)"
		;;
	
	*)
	echo "ERROR: This version of Debian: ${DEBIANVER} is not known by this script, please add it!"
	exit 1
esac

# Compile everithing
if [[ "$ACTION" == "C" || "$ACTION" == "c" || "$ACTION" == "" ]]; then
	# Get number of processors available
	PROCESSORS="$(grep -c ^processor /proc/cpuinfo)"

	# Build shared library and command line version
	make -j${PROCESSORS} all

	# Get version number from the compiled executable
	VERSION="$(./Release/airspaceconverter -v | head -n 1 | sed -r 's/^[^0-9]+([0-9]+.[0-9]+.[0-9]+).*/\1/g')"

	# Build Qt user interface
	mkdir -p buildQt
	cd buildQt
	qmake ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec linux-g++-64
	make -j${PROCESSORS} all
	cd ..
else
	# Ask the version to the user
	printf "New airspaceconverter version number: "
	read -r VERSION
fi

# Man page has to be compressed at maximum
gzip -9 < airspaceconverter.1 > airspaceconverter.1.gz

# Make folder for airspaceconverter
sudo rm -rf airspaceconverter_${VERSION}-${DEBIANVER}_${ARCH}
mkdir airspaceconverter_${VERSION}-${DEBIANVER}_${ARCH}
cd airspaceconverter_${VERSION}-${DEBIANVER}_${ARCH}

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
cp ../../Release/airspaceconverter ./bin
cp ../../Release/libairspaceconverter.so ./lib
cp ../../icons/* ./share/airspaceconverter/icons
mv ../../airspaceconverter.1.gz ./share/man/man1

cd ..

# Estimate package size
SIZE="$(du -s --apparent-size usr | awk '{print $1}')"

# Make control directory
mkdir DEBIAN
cd DEBIAN

# Make control file
echo 'Package: airspaceconverter
Version: '${VERSION}'-'${DEBIANVER}'
Section: misc
Priority: optional
Build-Depends: libc6-dev (>= '${LIBCVER}'), libboost-system-dev (>= '${BOOSTVER}'), libboost-filesystem-dev (>= '${BOOSTVER}'), libboost-locale-dev (>= '${BOOSTVER}'), libzip-dev (>= '${ZIPVER}')
Standards-Version: 3.9.4
Architecture: '${ARCH}'
Depends: libc6 (>= '${LIBCVER}'), libboost-system'${BOOSTVER}' (>= '${BOOSTVER}'), libboost-filesystem'${BOOSTVER}' (>= '${BOOSTVER}'), libboost-locale'${BOOSTVER}' (>= '${BOOSTVER}'), '${ZIPLIB}' (>= '${ZIPVER}')
Enhances: airspaceconverter-gui (>= '${VERSION}'), cgpsmapper (>= 0.0.9.3c)
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

#Root has to be the owner of all files
sudo chown -R root:root *
cd ..

#Make airspaceconverter DEB package
rm -f airspaceconverter_${VERSION}-${DEBIANVER}_${ARCH}.deb
dpkg-deb --build airspaceconverter_${VERSION}-${DEBIANVER}_${ARCH}

# Make folder for airspaceconverter-gui
sudo rm -rf airspaceconverter-gui_${VERSION}-${DEBIANVER}_${ARCH}
mkdir airspaceconverter-gui_${VERSION}-${DEBIANVER}_${ARCH}
cd airspaceconverter-gui_${VERSION}-${DEBIANVER}_${ARCH}

# Build directory structure
mkdir usr
cd usr
mkdir bin
mkdir share
cd share
mkdir applications
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
cd ..
mkdir doc
cd doc
mkdir airspaceconverter-gui
cd ..
mkdir menu
cd menu
echo '?package(airspaceconverter-gui): needs="X11"\
   section="Applications/Science/Geoscience"\
   title="Airspace Converter"\
   icon="/usr/share/pixmaps/airspaceconverter.xpm"\
   command="/usr/bin/airspaceconverter-gui"' > airspaceconverter-gui
cd ..
mkdir pixmaps
cd ..

# Copy the files
cp ../../airspaceconverter_${VERSION}-${DEBIANVER}_${ARCH}/usr/share/doc/airspaceconverter/copyright ./share/doc/airspaceconverter-gui
cp ../../airspaceconverter.xpm ./share/pixmaps
cp ../../buildQt/airspaceconverter-gui ./bin
strip -S --strip-unneeded ./bin/airspaceconverter-gui

cd ..

# Estimate package size
SIZE="$(du -s --apparent-size usr | awk '{print $1}')"

# Make control directory
mkdir DEBIAN
cd DEBIAN

# Make control file
echo 'Package: airspaceconverter-gui
Version: '${VERSION}'-'${DEBIANVER}'
Section: misc
Priority: optional
Standards-Version: 3.9.4
Architecture: '${ARCH}'
Depends: libc6 (>= '${LIBCVER}'), airspaceconverter (>= '${VERSION}'), '${QTDEPS}', libboost-system'${BOOSTVER}' (>= '${BOOSTVER}') ,libboost-filesystem'${BOOSTVER}' (>= '${BOOSTVER}')
Suggests: airspaceconverter (>= '${VERSION}')
Enhances: cgpsmapper (>= 0.0.9.3c)
Maintainer: Alberto Realis-Luc <admin@alus.it>
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
 contain the size of output files. This software, written entirely in C++, has a
 portable "core", it works from "command line" on Linux.
Homepage: http://www.alus.it/AirspaceConverter/
' > control

# Make post inst script
echo '#!/bin/sh
set -e
if [ "$1" = "configure" ] && [ -x "`which update-menus 2>/dev/null`" ]; then
	update-menus
fi
' > postinst
chmod a+x postinst

# Make post removal script
echo '#!/bin/sh
set -e
if [ -x "`which update-menus 2>/dev/null`" ]; then update-menus ; fi
' > postrm
chmod a+x postrm

cd ..

#Root has to be the owner of all files
sudo chown -R root:root *
cd ..

#Make DEB package for airspaceconverter-gui
rm -f airspaceconverter-gui_${VERSION}-${DEBIANVER}_${ARCH}.deb
dpkg-deb --build airspaceconverter-gui_${VERSION}-${DEBIANVER}_${ARCH}

# Clean
rm -R buildQt
sudo rm -R airspaceconverter_${VERSION}-${DEBIANVER}_${ARCH}
sudo rm -R airspaceconverter-gui_${VERSION}-${DEBIANVER}_${ARCH}
if [[ "$ACTION" == "D" || "$ACTION" == "d" ]]; then
	make clean
fi
exit 0

