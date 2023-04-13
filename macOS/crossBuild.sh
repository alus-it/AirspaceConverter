#!/bin/bash
# crossBuild.sh V0.02.00 2023/04/13 Copyright © 2021-2023 Valerio Messina
# License : GNU GPL v3
# cross-build AirspaceConverter from Linux to macOS
# Note: For the thing to work it is necessary to install 'osxcross'
# and need to be exported the env vars:
#       $(OCROSS)  cross-compiler tools prefix, eg. x86_64-apple-darwin18-
#       $(OSXROOT) point to cross-compiler root path, eg. /opt/osxcross
#       $(OSXDEPS) point to $(OSXROOT)/target/macports
# Note: $(OSXROOT)/target/bin   must be in PATH to find $(OCROSS)<tools>
#
echo "crossBuild.sh V0.02.00 2023/04/13 Copyright © 2021-2023 Valerio Messina"
cd ..
mkdir -p buildQt
cp macOS/Makefile.darwin-clang++ .
cp macOS/MakefileGui.darwin-clang++ buildQt
echo "CrossBuilding CLI and LIB ..."
make -f Makefile.darwin-clang++
echo ""
cd buildQt
cp -a MakefileGui.darwin-clang++ Makefile
echo "CrossBuilding GUI ..."
make -f MakefileGui.darwin-clang++
cp airspaceconverter-gui ../Release

# Note: once cross-built, to generate a compressed DMG on Linux, compile 'dmg' then:
# AirspaceConverter$ cp macOS/AirspaceConverter.icns Release/osxcross-dmg.icns
# AirspaceConverter$ cd Release
# AirspaceConverter/Release$ ../macOS/osxcross-dmg airspaceconverter AirspaceConverter
# AirspaceConverter/Release$ ../macOS/osxcross-dmg airspaceconverter-gui AirspaceConverter
#
# when asked, customize Info.plist, copy PNGs from 'icons' in:
# Release/DiskImage/AirspaceConverter.app/Contents/Resources/icons/
