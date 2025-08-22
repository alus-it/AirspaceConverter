#!/bin/bash
# makeAppImage.sh: Copyright 2024 Valerio Messina efa@iol.it
# makeAppImage is part of AirspaceConverter - convert different airspace/waypoint formats
# AirspaceConverter is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# AirspaceConverter is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with AirspaceConverter. If not, see <http://www.gnu.org/licenses/>.
#
# Script to generate a Linux AppImage portable package of 'AirspaceConverter'
# used on: Linux=>bin64, Linux=>bin32
#
# Syntax: $ makeAppImage.sh

makever=2024-05-22


DSTPATH="." # path where create the Linux package directory

echo "makeAppImage: Create a Linux AppImage package for AirspaceConverter ..."

# check for external dependency compliance
flag=0
for extCmd in cp cut grep mkdir mv rm tr uname ; do
   exist=`which $extCmd 2> /dev/null`
   if (test "" = "$exist") then
      echo "Required external dependency: "\"$extCmd\"" unsatisfied!"
      flag=1
   fi
done
if [[ "$flag" = 1 ]]; then
   echo "ERROR: Install the required packages and retry. Exit"
   exit
fi

if [[ "$1" != "" ]]; then
   echo "makeAppImage ERROR: no parameter are needed"
   echo "Syntax: $ makeAppImage.sh"
   exit
fi

PKG="Linux" # target OS from Make
CPU=`uname -m` # i686 or x86_64
if (test "" = "$2") then
   BIT=$(getconf LONG_BIT)
else
   BIT="$2"
fi
if (test "$CPU" = "x86_64" && test "$BIT" = "32") then
   CPU=i686
fi
OS=`uname` # current OS
if (test "$OS" != "Darwin") then
   OS=`uname -o`  # Msys or GNU/Linux, illegal on macOS
fi
VER=`grep "define VERSION" src/AirspaceConverter.hpp | cut -d' ' -f3 | tr -d '"'`
DATE=`date -I`
DIR=`pwd`
BIN=Release
DEP="" # optional deps
TGT=$PKG # target OS
DMG=""
DST="AirspaceConverter${VER}_${DATE}_${TGT}_${CPU}_${BIT}bit"

if [[ "$OS" != "GNU/Linux" ]]; then
   echo "ERROR: work in OS different than Linux"
   exit
fi

echo "PKG : $PKG"
echo "CPU : $CPU"
echo "BIT : $BIT"
echo "OS  : $OS"
echo "VER : $VER"
echo "DIR : $DIR"
echo "BIN : $BIN"
echo "DEP : $DEP"
echo "TGT : $TGT"
echo "DATE: $DATE"
echo "DST : $DSTPATH/$DST"
read -p "Proceed? A key to continue"
echo ""

echo "makeAppImage: Creating AirspaceConverter $VER package for $CPU $TGT $BIT bit ..."

rm -rf AppDir
mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/lib
mkdir -p AppDir/usr/share/man/man1
mkdir -p AppDir/usr/share/airspaceconverter/icons
mkdir -p AppDir/usr/share/pixmaps

# copy binaries:
cp -a $BIN/airspaceconverter       AppDir/usr/bin/
cp -a $BIN/airspaceconverter-gui   AppDir/usr/bin/
# copy libraries:
cp -a $BIN/libairspaceconverter.so AppDir/usr/lib/
# copy resources:
gzip -9 -n < airspaceconverter.1 > airspaceconverter.1.gz
mv airspaceconverter.1.gz          AppDir/usr/share/man/man1
cp -a icons/*                      AppDir/usr/share/airspaceconverter/icons
cp -a airspaceconverter.xpm        AppDir/usr/share/pixmaps
cp -a AirspaceConverter.png        AppDir/
cp -a LICENSE                      AppDir/
echo '[Desktop Entry]
Version=1.0
Type=Application
Name=AirspaceConverter
Comment=tool to convert between different airspace and waypoint formats
Exec=airspaceconverter-gui
Terminal=false
StartupNotify=false
Icon=AirspaceConverter
Categories=Education;Science;Geoscience' > airspaceconverter-gui.desktop
cp -a airspaceconverter-gui.desktop AppDir/

if (test "$PKG" = "Linux" && (test "$CPU" = "x86_64" || test "$CPU" = "i686")) then # skip on ARM&RISC-V
   echo "makeAppImage: Generating the AppImage for AirspaceConverter (about 1') ..."
   if (test -f logWget$DATE.txt) then { rm logWget$DATE.txt ; } fi
   if (test "$BIT" = "64") then
      if (! test -x linuxdeploy-x86_64.AppImage) then
         wget -nv "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" 2>> logWget$DATE.txt
         chmod +x linuxdeploy-x86_64.AppImage
      fi
      if (! test -x linuxdeploy-plugin-qt-x86_64.AppImage) then
         wget -nv "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" 2>> logWget$DATE.txt
         chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
      fi
      if (! test -x linuxdeploy-plugin-appimage-x86_64.AppImage) then
         wget -nv "https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-x86_64.AppImage" 2>> logWget$DATE.txt
         chmod +x linuxdeploy-plugin-appimage-x86_64.AppImage
      fi
      rm -f AirspaceConverter-x86_64.AppImage 2> /dev/null
      linuxdeploy-x86_64.AppImage -e $BIN/airspaceconverter-gui --appdir AppDir -p qt -i AirspaceConverter.png -d airspaceconverter-gui.desktop --output appimage > logLinuxdeploy$DATE.txt
      if (test -f AirspaceConverter-x86_64.AppImage) then
         file=$DST.AppImage
         mv AirspaceConverter-x86_64.AppImage $file
         echo "AppImage created: $file"
      else
         echo "AppImage creation error, see log: logLinuxdeploy$DATE.txt"
      fi
   fi
   if (test "$BIT" = "32") then
      echo "As now skip AppImage at 32 bit"
   fi
fi
read -p "Delete tmp file and AppDir directory? (y/n) " ret
#echo "ret:$ret"
if (test "$ret" != "n") then
   rm airspaceconverter-gui.desktop
   rm -rf AppDir
fi
