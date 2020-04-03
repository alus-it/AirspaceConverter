AirspaceConverter
=================
A multi-platform and open source tool to convert between different airspace and waypoint formats.  

Can read airspace and waypoint files in the following formats:  
  - _OpenAir_  
  - _openAIP_  
  - KML/KMZ  
  - _SeeYou_  

Can write in the following formats:  
  - KMZ  
  - _OpenAir_  
  - _SeeYou_  
  - _Polish_  
  - _Garmin_ IMG  

While converting to _OpenAir_ AirspaceConverter estimates if the points are part of arcs or circumferences in order to make use of arc and circumference definitions of the _OpenAir_ format and so avoiding to output all points one by one.  
The ability to read **KML**/**KMZ** is based on the **KMZ** airspace files produced by _Austrocontrol_. This utility can convert also _SeeYou_ **.CUP** waypoint files to **KMZ** (for _Google Earth_). The conversion to **IMG** for _Garmin_ devices is done using _cGPSmapper_.  
AirspaceConverter is written in C++11 and runs on _Linux_, _Windows_ and _macOS_. In order to be immediately easy to use it has _Qt_ and _Windows MFC_ user interfaces. But it can also work from command line.  


For more information's about this project: [alus.it/AirspaceConverter](https://www.alus.it/AirspaceConverter)  
Contributors are, always, warmly welcome!

Why this software?
------------------
Airspace represented in 3D in _Google Earth_ can help to visualize and better understand the airspace structure.
Not only, having the planned route or track of a flight (as GPX file), displayed as well in _Google Earth_ will make easy to check for airspace crossed or to be avoided.  
This software can also be useful for maintainers of _OpenAir_ airspace and _SeeYou_ waypoints files, not only to visualize airspace and waypoints but also to verify the syntax of _OpenAir_ or _SeeYou_ lines entered.  
Duplicate consecutive points will be ignored, the converter will warn about them while reading _OpenAir_ files. This will also detect the special case of an unnecessary point repeating the end of the arc defined on the previous line.  
For the "landable" waypoints in the **CUP** files an estimation of the runway perimeter is drawn on the earth surface, in order to do that not only the position is used but also the runway orientation and length. This is particularly useful to verify that the position of the airfield in the **CUP** file matches exactly the runway in _Google Earth_.  In software like _LK8000_ the airfield position is considered as the exact center of the runway, orientation and length are used to assist for landing with an HSI and glide slope indications thus the correctness of position, orientation and length becomes really important.  
_openAIP_ (http://www.openaip.net/) provides a free, worldwide and updated airspace and waypoint repository but in his own format, while many devices and software support _OpenAir_ airspace files. This software can convert: _openAIP_ airspace to _OpenAir_ and _openAIP_ waypoints to _SeeYou_; so making _openAIP_ data available to many portable devices.  
AirspaceConverter can be used also to merge together several airspace or waypoint files (_openAIP_ repeated airspaces will be automatically removed) and filter the result on a specific range of latitudes and longitudes.  
Converting _openAIP_ files to _OpenAir_ has also the advantage to reduce significantly the size of the total airspace database used on a portable device. For example _LK8000_ recently supports also the _openAIP_ format (feature that I implemented) but it is not always possible to load bigger _openAIP_ files on older PNA devices, while the same files converted in _OpenAir_ yes.  
There are also cases where the "official" airspace files are available only in KML format like the Austrian airspace from _Austrocontrol_, also in this case is possible to convert it to _OpenAir_.  
In case is required to import long lists of points (like state borders) from KML LineString tracks: just use the option `-t` and the tracks found will be closed and treated as unknown airspace. Then it will be possible to adapt the airspace definitions manually in the so converted _OpenAir_ file.  

About KML format
----------------
The _[Keyhole Markup Language](https://developers.google.com/kml/)_, used by _Google Earth_ is probably a good format to define models of buildings but it is not exactly practical to define airspace. This because it not possible, at least from my understanding, to define in the same 3D object points at altitudes with different references: it is the typical case of an airspace with AGL floor and AMSL ceiling.  
One possible solution is to translate, where necessary all the altitudes to the same reference but to do that we need the know the altitude of the terrain at every location, reason why this software uses terrain maps.  
Another important limitation of KML is that surfaces which follow the terrain are possible only at terrain altitude.  
So please, be advised that, what you will see in _Google Earth_ will not exactly match how airspace definitions are really intended.  

LK8000 terrain raster map files
-------------------------------
This program uses the same terrain raster maps (**.dem**) of _LK8000_ to convert altitudes from AGL to AMSL.  
In case the loaded terrain maps are overlapping, the one with the best resolution will be automatically used.  
On the _LK8000_ website there is a [wide terrain raster map collection](https://www.lk8000.it/download/maps.html).  
The part of this program reading and using the terrain maps has been derived and adapted from _LK8000_ project.  
To know more about _LK8000_ project: [lk8000.it](https://www.lk8000.it/)  
The repository of _LK8000_ is available here: [github.com/LK8000](https://github.com/LK8000/LK8000)  

AirspaceConverter shared library
--------------------------------
This software is designed to keep completely separated functionalities from the user interfaces.  
All the reading, writing and conversion features are in a common shared library: libAirspaceConverter can be compiled under different platforms and so used from different user interfaces.  

AirspaceConverter command line
------------------------------
The `airspaceconverter` command line executable, works taking several arguments, for example:  
`airspaceconverter -q 1013 -a 35 -i inputFileOpenAir.txt -i openAIP_asp.aip -w waypoints.cup -w openAIP_wpt.aip -m terrainMap.dem -o outputFile.kmz`  

Possible options:  
  - **-q**: optional, specify the QNH in hPa used to calculate height of flight levels  
  - **-a**: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)  
  - **-i**: multiple, input file(s) can be _OpenAir_ (**.txt**), _openAIP_ (**.aip**), _Google Earth_ **.kmz**, **.kml**)  
  - **-w**: multiple, input waypoint file(s) can be _SeeYou_ (**.cup**) or _openAIP_ (**.aip**)  
  - **-m**: optional, multiple, terrain map file(s) (**.dem**) used to lookup terrain heights  
  - **-l**: optional, set filter limits in latitude and longitude for the output, followed by the 4 limit values: northLat,southLat,westLon,eastLon where the limits are comma separated, expressed in degrees, without spaces, negative for west longitudes and south latitudes  
  - **-o**: optional, output file **.kmz**, **.txt** (_OpenAir_), **.cup** (_SeeYou_), **.img** (_Garmin_) or **.mp** (_Polish_). If not specified will be used the name of first input file as **KMZ**  
  - **-p**: optional, when writing in _OpenAir_ avoid to use arcs and circles but only points (DP)  
  - **-s**: optional, when writing in _OpenAir_ use coordinates always with minutes and seconds (DD:MM:SS)  
  - **-d**: optional, when writing in _OpenAir_ use coordinates always with decimal minutes (DD:MM.MMM)  
  - **-t**: optional, when reading **KML**/**KMZ** files treat also "LineString" tracks as airspaces  
  - **-v**: print version number  
  - **-h**: print short guide  

At least one input airspace or waypoint file must be present.  
Warning: any already existing output file will be overwritten.  

Graphical user interface
------------------------
In order to be portable, the graphical interface is written in _Qt_ and actually it can be compiled in Linux, Windows and macOS.  
For Windows only is also available a _Windows MFC_ user interface; if, while starting it, you get the error about `VCRUNTIME140.dll` missing: it can be easily fixed installing the: _Microsoft VC++ redistributable_, this will allow this software to run also on older Windows versions such us _WindowsXP_.  
This is the default way to use the graphical user interface:  
1. Choose the desired output format.
2. If needed, specify the QNH to be used for calculating the height of flight levels, this must be done before reading airspace files.
3. Specify a default terrain altitude to be used for the points not under terrain raster map coverage.
4. Select as input multiple _openAIP_ (**.aip**) _OpenAir_ (**.txt**) and/or _GoogleEarth_ (**.kmz**) files or the folder containing them.
5. And/or select one or multiple waypoints files (**.cup**) or the folder containing them.
6. If converting to _GoogleEarth_ it is possible to load multiple raster map files (**.dem**) with the terrain altitude.
7. Optionally configure the latitude and longitude ranges for filtering the output.
8. If converting to _OpenAir_ choose if to output only points and the desired coordinates format.
8. Press the "Convert" button.
9. The converter will ask where to save the converted file and then the conversion process will start.
10. Verify if the output is correct and report any problem found.

Disclaimer
----------
**WARNING**: this program is experimental. The generated output files may contains errors.  
So please always verify the generated files before using in flight and report any error found.  
By using this program you understand and completely agree that the generated output files (maybe wrong) are just for demonstration purposes and they do not absolutely substitute the official AIP publications.  
Please always refer to official AIP publications for valid and updated airspace definitions.  

Downloads
---------
The already compiled executables are available to download from the [project downloads page](https://www.alus.it/AirspaceConverter/download.php)  
For the Linux distributions based on _Debian_ there is a also an _APT_ repository available.  

Build dependencies
------------------
In order to compile this project the following libraries are required:  
- Qt (`libqt4-dev`)
- libzip2 (`libzip-dev`) and dependencies (zlib)
- Boost libraries (`libboost-filesystem-dev` and `libboost-locale-dev`)  

Compiling and installing AirspaceConverter from sources on Linux
----------------------------------------------------------------
First it is necessary to install the dependencies, on a _Debian_ based distribution it would be:  
`sudo apt install libzip-dev libboost-filesystem-dev libboost-locale-dev libqt4-dev`  
Then, to compile, from the root of this project: `./build.sh`  
To install: `./install.sh`  
This will install everything: the shared library the command line executable and the _Qt_ GUI interface.  
After it will be possible to run AirspaceConverter CLI from anywhere simply calling: `airspaceconverter`  
The same for the AirspaceConverter _Qt_ GUI graphical interface: `airspaceconverter-gui`  
To uninstall (from the root of the project): `./uninstall.sh`  

Compiling and installing AirspaceConverter from sources on macOS
----------------------------------------------------------------
First it is necessary to install _Qt_ and the other dependencies:  
`brew install libzip boost`  
Make sure to have the Qt tools reachable from your `$PATH`. This can be done, for example, adding to `.bash_profile` a line like:  
`export PATH="$HOME/Qt/<Qt version>/clang_64/bin":"$PATH"`  
Then, to compile and install, from the root of this project: `./install.sh`  
This will install everything: the shared library the command line executable and the _Qt_ GUI interface.  
After it will be possible to run AirspaceConverter CLI from anywhere simply calling: `airspaceconverter`  
While the AirspaceConverter _Qt_ GUI graphical interface will be directly available from the Launchpad menu.  
To uninstall (from the root of the project): `./uninstall.sh`  

Compiling AirspaceConverter on Windows
--------------------------------------
In the "VisualStudio" folder there is the VisualStudio solution and project files.  
It requires to have the proper Boost libraries installed, with their location configured in the VS project.  
From [SourceForge](https://sourceforge.net/projects/boost/files/boost-binaries/) download the latest version of Boost libraries alredy compiled for VisualStudio.  
While libzip and zlib (also not included in this repository) can be obtained via nuget.  
When compiling with VisualStudio 2017 (vs141), and newer, in order to link with libzip it is necessary to modify `libzip.targets` located in `VisualStudio\packages\libzip.1.1.2.7\build\native` replacing all occurrencies of `PlatformToolset.ToLower().IndexOf('v140')` with `PlatformToolset.ToLower().IndexOf('v14')`  
If required _cGPSmapper_ can be found in the portable distribution archive of this project.  

Placemarks icons credits
------------------------
Some of the placemark icons used for displaying the waypoints in _Google Earth_ included in the produced KMZ file (and so used by this project) are coming from: [Maps Icons Collection](https://mapicons.mapsmarker.com)  
On Windows, the folder `icons` with the placemarks PNG icons must be kept in the same location of AirspaceConverter executable.  

Contributors
------------
- _[Valerio Messina](mailto:efa@iol.it)_ : packaging for _Ubuntu_, various issues fixed and testing

Contacts
--------
Author: _Alberto Realis-Luc_  
Web: [alus.it/AirspaceConverter](https://www.alus.it/AirspaceConverter/)  
E-mail: [info@alus.it](mailto:info@alus.it?subject=AirspaceConverter)  
