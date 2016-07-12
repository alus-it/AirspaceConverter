AirspaceConverter
=================

Welcome to AirspaceConverter repository!
This is an open source, multiplatform  tool to convert OpenAir and OpenAIP airspace files to KML/KMZ for Google Earth.
This software, written entirely in C++11 can be ran both on Linux and on Windows.
The Windows version has MFC user interface in order to make this software easy to use and immediately usable.
For more informations about this project: http://www.alus.it/AirspaceConverter

Contributors are, as usual, warmly welcome!


Why?
====

Airspace represented in 3D in Google Earth can help to visualize and better understand the airspace structure.
Not only, having the planned route or track of a flight (as GPX file), displayed as well in Google Earth will make easy to check for airspace crossed or to be avoided.


KML format
==========

KML, used by Google Earth is probably a good format to define models of buildings but it is not exactly practical to define airspace. This because it not possible, at least from my understanding, to define in the same 3D object points at altitudes with different references: it is the typical case of an airspace with AGL floor and AMSL ceiling.
One possible solution is to translate, where necessary all the altitudes to the same reference but to do that we need the know the altitude of the terrain at every location.
AirspaceConverter uses the .DEM terrain raster map of LK8000 to get the altitude of the terrain.
Another limitation of KML is that surfaces which follow the terrain are possible only at terrain altitude.


How to use it
=============

1. If needed, specify the QNH to be used for calculating the height of flight levels.
2. Select as input multiple OpenAir (.txt) and/or OpenAIP (.aip) files.
3. Optionally it is possible to load multiple raster map files (.dem) with the terrain altitude.
4. Specify a default terrain altitude to be used for the points not under terrain raster map coverage.
5. The output can be done in KML or directly compressed as KMZ.
6. Verify if the output is correct and report any problem found.

On the LK8000 website there is a wide terrain raster map collection: http://www.lk8000.it/download/maps.html


Linux version
=============

To be done...


Windows version
===============

The Windows executable needs just a couple of small DLLs: zip.dll zlib.dll to be kept in the same folder.
If you get the error about VCRUNTIME140.dll missing: it can be easily and quickly fixed installing the: MS VC++ redistributable, this will allow this software to run also on older Windows versions such us WindowsXP.


Downloads
=========

The already compiled executable for windows can be downloaded from the project page:
http://www.alus.it/AirspaceConverter


Preparing the development environment
=====================================

To be done....



Compiling AirspaceConverter
===========================

To be done....



CONTACTS
========

Author: Alberto Realis-Luc
Web: http://www.alus.it/AirspaceConverter/
E-mail: info@alus.it
