AirspaceConverter
=================
This is a free, open source and multi-platform tool to convert between different airspace formats.  

Can read airspace files in the following formats:  
  - OpenAir  
  - OpenAIP  
  - KML/KMZ  

And the output can be done in the following formats:  
  - KMZ  
  - OpenAir  
  - Polish  
  - Garmin IMG  

This utility can convert also SeeYou .CUP waypoint files to KMZ (for Google Earth). The conversion to IMG for Garmin devices is done using cGPSmapper.  
While converting to OpenAir this software estimates if the points entered are part of arcs or circumferences in order to make use of arc and circumference definitions of the OpeanAir format and so avoiding to output all points one by one.  
The ability to read KML/KMZ is based on the KMZ airspace files produced by Austrocontrol.  
AirspaceConverter is written in C++11 can be ran both on Linux and on Windows. It has a Qt user interface and Windows MFC UI as well in order to be immediately easy to use. Under Linux can also work also from command line to be usable from shell scripts.  
For more information's about this project: http://www.alus.it/AirspaceConverter  
Contributors are, as usual, warmly welcome!

Why this software?
------------------
Airspace represented in 3D in Google Earth can help to visualize and better understand the airspace structure.
Not only, having the planned route or track of a flight (as GPX file), displayed as well in Google Earth will make easy to check for airspace crossed or to be avoided.  
This software can also be useful for maintainers of OpenAir airspace and SeeYou .CUP waypoints files, not only to visualize airspace and waypoints but also to verify the syntax of OpenAir and CUP commands entered.  
For the "landable" waypoints in the CUP files an estimation of the runway perimeter is drawn on the earth surface, in order to do that not only the position is used but also the runway orientation and length. This is particularly useful to verify that the position of the airfield in the CUP file matches exactly the runway in Google Earth.  In software like LK8000 the airfield position is considered as the exact center of the runway, orientation and length are used to assist for landing with an HSI and glide slope indications thus the correctness of position, orientation and length becomes really important.  
OpenAIP (http://www.openaip.net/) provides a free, worldwide and updated airspace repository but in his own format, while many devices and software support OpenAir airspace files. This software can convert also to OpenAir so making OpenAIP data available to many portable devices.  
AirspaceConverter can be used also to merge together several airspace files (OpenAIP repeated airspaces will be automatically removed) and filter the result on a specific range of latitudes and longitudes.  
There are also cases where the "official" airspace files are available only in KML format like the Austrian airspace from Austrocontrol, also in this case is possible to convert it to OpenAir.

About KML format
----------------
KML, used by Google Earth is probably a good format to define models of buildings but it is not exactly practical to define airspace. This because it not possible, at least from my understanding, to define in the same 3D object points at altitudes with different references: it is the typical case of an airspace with AGL floor and AMSL ceiling.  
One possible solution is to translate, where necessary all the altitudes to the same reference but to do that we need the know the altitude of the terrain at every location, reason why this software uses terrain maps.  
Another important limitation of KML is that surfaces which follow the terrain are possible only at terrain altitude.  
So please, be advised that, what you will see in Google Earth will not exactly match how airspace definitions are really intended.

LK8000 terrain raster map files
-------------------------------
This program uses the same terrain raster maps (.dem) of LK8000 to convert altitudes from AGL to AMSL.  
In case the loaded terrain maps are overlapping, the one with the best resolution will be automatically used.  
On the LK8000 website there is a wide terrain raster map collection: http://www.lk8000.it/download/maps.html  
The part of this program reading and using the terrain maps has been derived and adapted from LK8000 project.  
To know more about LK8000 project: http://www.lk8000.it  
The repository of LK8000 is available here: https://github.com/LK8000/LK8000

The common shared library
-------------------------
This software is designed to keep completely separated functionalities from the user interfaces.  
All the reading, writing and conversion features are in a common shared library: libAirspaceConverter can be compiled under different platfoms and so used from different user interfaces.

Command line version
--------------------
The command line version, compiled both for Linux and Windows, works taking several arguments, for example:  
`AirspaceConverter -q 1013 -a 35 -i inputFileOpenAir.txt -i inputFileOpenAIP.aip -w waypoints.cup -m terrainMap.dem -o outputFile.kmz`  

Possible options:  
  -q: optional, specify the QNH in hPa used to calculate height of flight levels  
  -a: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)  
  -i: multiple, input file(s) can be OpenAir (.txt), OpenAIP (.aip), Google Earth (.kmz, .kml)  
  -w: multiple, input waypoint file(s) in the SeeYou CUP format (.cup)  
  -m: optional, multiple, terrain map file(s) (.dem) used to lookup terrain heights  
  -l: optional, set filter limits in latitude and longitude for the output, followed by the 4 limit values: northLat,southLat,westLon,eastLon where the limits are comma separated, expressed in degrees, without spaces, negative for west longitudes and south latitudes  
  -o: optional, output file .kmz, .mp (Polish) or .txt (OpenAir) if not specified will be used the name of first input file as KMZ  
  -v: print version number  
  -h: print this guide  

At least one input airspace or waypoint file must be present.  
Warning: any already existing output file will be overwritten.

Windows MFC version
-------------------
The native Windows MFC executable needs just a couple of small DLLs: zip.dll zlib.dll to be kept in the same folder.  
If you get the error about VCRUNTIME140.dll missing: it can be easily and quickly fixed installing the: MS VC++ redistributable, this will allow this software to run also on older Windows versions such us WindowsXP.  

This Windows MFC version has graphical user interface, this should be the default way to use it:

1. Choose the desired output format
2. If needed, specify the QNH to be used for calculating the height of flight levels, this must be done before reading airspace files.
3. Specify a default terrain altitude to be used for the points not under terrain raster map coverage.
4. Select as input multiple OpenAir (.txt) and/or OpenAIP (.aip) files or the folder containing them.
5. And/or select one or multiple waypoints files (.cup) or the folder containing them.
6. Optionally it is possible to load multiple raster map files (.dem) with the terrain altitude.
7. Optionally configure the latitude and longitude ranges for filtering the output.
8. Eventually choose a different name for the output file.
9. Press the button convert to start the conversion process.
10. Verify if the output is correct and report any problem found.

Qt user interface
-----------------
In order to be portable, the graphical interface has been rewritten also in Qt and actually it can be compiled both under Linux and Windows.

Disclaimer
----------
WARNING: this program is experimental. The generated output files may contains errors.
So please always verify the generated files before using in flight and report any error found.  
By using this program you understand and completely agree that the generated output files (maybe wrong) are just for demonstration purposes and they do not absolutely substitute the official AIP publications.  
Please always refer to official AIP publications for valid and updated airspace definitions.

Downloads
---------
The already compiled executables for Windows can be downloaded from the project page:  
http://www.alus.it/AirspaceConverter
For the Linux distributions based on Debian there is a also repository available. 

Build dependencies
------------------
In order to compile this project the following libraries are required:  
- libzip2 (libzip-dev) and dependencies (zlib)
- Boost libraries (libboost-filesystem-dev and libboost-locale-dev)

Compiling and installing AirspaceConverter from sources on Linux
----------------------------------------------------------------
First it is necessary to install the dependencies, on a Debian based distribution it would be:  
`sudo apt-get install libzip-dev libboost-filesystem-dev libboost-locale-dev`  
Then, to compile, from the root of this project: `make all`  
To install: `sudo make install`  
This will install the shared library and the AirspaceConverter command line executable.  
After it will be possible to run AirspaceConverter from anywhere simply calling: `airspaceconverter`  
To uninstall (from the root of the project): `sudo make uninstall` 

Compiling the Qt GUI executable
-------------------------------
In the "AirspaceConverterQt" folder there is the Qt project, it needs the shared library libAirspaceConverter already compiled and installed for the same platform, as described in the previous step.  
It is also possible to build from command line, first make sure to have the Qt development environment installed: `sudo apt-get install libqt4-dev`  
Then to compile in the Qt user interface in the "buildQt" folder:
`mkdir buildQt`  
`cd buildQt`  
`qmake ../AirspaceConverterQt/AirspaceConverterQt.pro -r -spec linux-g++-64`  
`make all`

Compiling AirspaceConverter with MFC interface on Windows
---------------------------------------------------------
In the "VisualStudio" folder there is the VisualStudio solution and project files.  
It requires to have the boost libraries installed, with their location configured in the VS project, while libzip can be obtained via nuget.

Placemarks icons credits
------------------------
Some of the placemark icons used for displaying the waypoints in Google Earth included in the produced KMZ file (and so used by this project) are coming from:  
Maps Icons Collection - https://mapicons.mapsmarker.com  
On Windows, the folder `icons` with the placemarks PNG icons must be kept in the same location of AirspaceConverter executable.

Contributors
------------
- Valerio Messina, efa@iol.it: packaging for Ubuntu 16.04, various issues fixed and testing

Contacts
--------
Author: Alberto Realis-Luc  
Web: http://www.alus.it/AirspaceConverter/  
E-mail: info@alus.it
