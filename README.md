AirspaceConverter
=================
Welcome to AirspaceConverter repository!  
This is an open source, multi-platform  tool to convert OpenAir or OpenAIP airspace files and SeeYou .CUP waypoint files to KMZ (for Google Earth). The airspace can be converted also back to OpenAir or to PFM "Polish" format, as file .mp, and then using cGPSmapper also to .img files for Garmin devices.  
While converting to OpenAir this software estimates if the points entered are part of arcs or circumferences in order to make use of arc and circumference definitions of the OpeanAir format and so avoiding to output all points one by one.  
AirspaceConverter is written in C++11 can be ran both on Linux and on Windows.
The Windows version has MFC user interface in order to be immediately easy to use, while the Linux version works from command line to be usable also from shell scripts.  
For more information's about this project: http://www.alus.it/AirspaceConverter  
Contributors are, as usual, warmly welcome!

Why this software?
------------------
Airspace represented in 3D in Google Earth can help to visualize and better understand the airspace structure.
Not only, having the planned route or track of a flight (as GPX file), displayed as well in Google Earth will make easy to check for airspace crossed or to be avoided.  
This software can also be useful for maintainers of OpenAir airspace and SeeYou .CUP waypoints files, not only to visualize airspace and waypoints but also to verify the syntax of OpenAir and CUP commands entered.  
For the "landable" waypoints in the CUP files an estimation of the runway perimeter is drawn on the earth surface, in order to do that not only the position is used but also the runway orientation and length. This is particularly useful to verify that the position of the airfield in the CUP file matches exactly the runway in Google Earth.  In software like LK8000 the airfield position is considered as the exact center of the runway, orientation and length are used to assist for landing with an HSI and glide slope indications thus the correctness of position, orientation and length becomes really important.  
OpenAIP (http://www.openaip.net/) provides a free, worldwide and updated airspace repository but in his own format, while many devices and software support OpenAir airspace files.
This software can convert also to OpenAir so making OpenAIP data available to many portable devices.

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

Linux version
-------------
The command line version works taking several arguments, for example:  
`AirspaceConverter -q 1013 -a 35 -i inputFileOpenAir.txt -i inputFileOpenAIP.aip -m terrainMap.dem -o outputFile.kmz`  

Possible options:  
  -q: optional, specify the QNH in hPa used to calculate height of flight levels  
  -a: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)  
  -i: mandatory, multiple, input file(s) can be OpenAir (.txt), OpenAIP (.aip) or SeeYou waypoints (.cup)  
  -m: optional, multiple, terrain map file(s) (.dem) used to lookup terrain height  
  -o: optional, output file .kmz, .mp (Polish) or .txt (OpenAir) if not specified will be used the name of first input file as KMZ  
  -v: print version number  
  -h: print this guide

Windows version
---------------
The Windows executable needs just a couple of small DLLs: zip.dll zlib.dll to be kept in the same folder.  
If you get the error about VCRUNTIME140.dll missing: it can be easily and quickly fixed installing the: MS VC++ redistributable, this will allow this software to run also on older Windows versions such us WindowsXP.  

The Windows version has graphical user interface, this should be the default way to use it:

1. Choose the desired output format
2. If needed, specify the QNH to be used for calculating the height of flight levels, this must be done before reading airspace files.
3. Specify a default terrain altitude to be used for the points not under terrain raster map coverage.
4. Select as input multiple OpenAir (.txt) and/or OpenAIP (.aip) files or the folder containing them.
5. And/or select one or multiple waypoints files (.cup) or the folder containing them.
6. Optionally it is possible to load multiple raster map files (.dem) with the terrain altitude.
7. Eventually choose a different name for the output file.
8. Press the button convert to start the conversion process.
9. Verify if the output is correct and report any problem found.

Disclaimer
----------
WARNING: this program has been still not fully tested. The generated output files may contains errors.
In particular the output in OpenAir is untested, so please always verify the generated files before using in flight and report any error found.  
By using this program you understand and completely agree that the generated output files (maybe wrong) are just for demonstration purposes and they do not absolutely substitute the official AIP publications.  
Please always refer to official AIP publications for valid and updated airspace definitions.

Downloads
---------
The already compiled executables for Linux and Windows can be downloaded from the project page:  
http://www.alus.it/AirspaceConverter

Build dependencies
------------------
In order to compile this project the following libraries are required:  
- libzip2 (libzip-dev) and dependencies (zlib)
- Boost libraries

Compiling AirspaceConverter on Linux
------------------------------------
From the root of this project: `make all`  
To build in debug: `make DEBUG=1 all`  
To build without zip support (in case the right libzip2 isn't available): `make NOZIP=1 all`  
The "AirspaceConverter" executable will be located in the "Release" or "Debug" folder

Compiling AirspaceConverter on Windows
--------------------------------------
In the "VisualStudio" folder there is the VisualStudio solution and project files

Placemarks icons credits
------------------------
Some of the placemark icons used for displaying the waypoints in Google Earth included in the produced KMZ file (and so used by this project) are coming from:  
Maps Icons Collection - https://mapicons.mapsmarker.com  
The folder icons with the placemarks PNG icons must be kept in the same location of AirspaceConverter executable.

Contacts
--------
Author: Alberto Realis-Luc  
Web: http://www.alus.it/AirspaceConverter/  
E-mail: info@alus.it
