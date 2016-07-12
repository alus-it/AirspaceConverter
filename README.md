AirspaceConverter
=================
Welcome to AirspaceConverter repository!  
This is an open source, multi-platform  tool to convert OpenAir or OpenAIP airspace files to KML/KMZ for Google Earth.  
AirspaceConverter is written entirely in C++11 can be ran both on Linux and on Windows.
The Windows version has MFC user interface in order to be easy to use and immediately usable.
For more information's about this project: http://www.alus.it/AirspaceConverter  
Contributors are, as usual, warmly welcome!

Why this software?
-----------------
Airspace represented in 3D in Google Earth can help to visualize and better understand the airspace structure.
Not only, having the planned route or track of a flight (as GPX file), displayed as well in Google Earth will make easy to check for airspace crossed or to be avoided.  
This software can also be useful for maintainers of OpenAir files, not only to visualize airspace but also to verify the syntax of OpenAir commands entered.

About KML format
----------------
KML, used by Google Earth is probably a good format to define models of buildings but it is not exactly practical to define airspace. This because it not possible, at least from my understanding, to define in the same 3D object points at altitudes with different references: it is the typical case of an airspace with AGL floor and AMSL ceiling.  
One possible solution is to translate, where necessary all the altitudes to the same reference but to do that we need the know the altitude of the terrain at every location.  
Another limitation of KML is that surfaces which follow the terrain are possible only at terrain altitude.

LK8000 terrain raster map files
-------------------------------
This program uses the same terrain raster maps (.dem) of LK8000 to convert altitudes from AGL to ASML.  
On the LK8000 website there is a wide terrain raster map collection: http://www.lk8000.it/download/maps.html  
The part of this program reading and using the terrain maps has been derived and adapted from LK8000 project.  
To know more about LK8000 project: http://www.lk8000.it  
The repository of LK8000 is available here: https://github.com/LK8000/LK8000

Linux version
-------------
The command line version works taking several arguments, for example:  
`AirspaceConverter -q 1013 -a 35 -i inputFileOpenAir.txt -i inputFileOpenAIP.aip -m terrainMap.dem -o outputFile.kmz`  

Possible options:  
* -q: optional, specify the QNH in hPa used to calculate height of flight levels
* -a: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)
* -i: mandatory, multiple, input file(s) can be OpenAir (.txt) or OpenAIP (.aip)
* -m: optional, multiple, terrain map file(s) (.dem) used to lookup terrain height
* -o: optional, output file .kml or .kmz if not specified will be used the name of first input file as KMZ
* -v: print version number
* -h: print this guide

Windows version
---------------
The Windows executable needs just a couple of small DLLs: zip.dll zlib.dll to be kept in the same folder.  
If you get the error about VCRUNTIME140.dll missing: it can be easily and quickly fixed installing the: MS VC++ redistributable, this will allow this software to run also on older Windows versions such us WindowsXP.  

The Windows version has graphical user interface, this should be the default way to use it:  
1. If needed, specify the QNH to be used for calculating the height of flight levels.
2. Select as input multiple OpenAir (.txt) and/or OpenAIP (.aip) files.
3. Optionally it is possible to load multiple raster map files (.dem) with the terrain altitude.
4. Specify a default terrain altitude to be used for the points not under terrain raster map coverage.
5. The output can be done in KML or directly compressed as KMZ.
6. Verify if the output is correct and report any problem found.

Disclaimer
----------
WARNING: this program has been still not fully tested. The generated output files may contains mistakes.  
By using this program you agree that the generated output files are just for demonstration purposes and they do not absolutely substitute the official AIP publications.  
Please refer to official AIP publications for valid and updated airspace definitions.

Downloads
---------
The already compiled executable for windows can be downloaded from the project page:  
http://www.alus.it/AirspaceConverter

Preparing the development environment
-------------------------------------
To be done....

Compiling AirspaceConverter
---------------------------
To be done....

CONTACTS
--------
Author: Alberto Realis-Luc  
Web: http://www.alus.it/AirspaceConverter/  
E-mail: info@alus.it
