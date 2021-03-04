#!/bin/bash
# crossBuild.sh V0.01.00 2021/03/03 Copyright © 2021 Valerio Messina
# License : GNU GPL v3
# cross-build AirspaceConverter from Linux to macOS
# Note: For the thing to work it is necessary to install 'osxcross'
# and need to be exported the env vars:
#       ${OSX}    point to cross-compiler environment path
#       ${OCROSS} cross-compiler tools prefix, eg. x86_64-apple-darwin18-
#       PATH=${OSX}/target/bin/${OCROSS}tools:$PATH
#
echo "crossBuild.sh V0.01.00 2021/03/03 Copyright © 2021 Valerio Messina"
cd ..
cp macOS/Makefile.darwin-clang++ .
cp macOS/MakefileGui.darwin-clang++ buildQt
make -f Makefile.darwin-clang++
cd buildQt
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
