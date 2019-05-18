//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "AirspaceConverter.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <boost/tokenizer.hpp>
#ifdef _WIN32
#include <boost/filesystem/operations.hpp>
#endif

void printHelp() {
	std::cout << "Example usage: airspaceconverter -q 1013 -a 35 -i inputFileOpenAir.txt -i inputFileOpenAIP.aip -w waypoints.cup -m terrainMap.dem -o outputFile.kmz" << std::endl << std::endl;
	std::cout << "Possible options:" << std::endl;
	std::cout << "-q: optional, specify the QNH in hPa used to calculate height of flight levels" << std::endl;
	std::cout << "-a: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)" << std::endl;
	std::cout << "-i: multiple, input airspace file(s) can be OpenAir (.txt), openAIP (.aip), Google Earth (.kmz, .kml)" << std::endl;
	std::cout << "-w: multiple, input waypoint file(s) in the SeeYou CUP format (.cup, .aip)" << std::endl;
	std::cout << "-m: optional, multiple, terrain map file(s) (.dem) used to lookup terrain heights" << std::endl;
	std::cout << "-l: optional, set filter limits in latitude and longitude for the output, followed by the 4 limit values: northLat,southLat,westLon,eastLon" << std::endl;
	std::cout << "    where the limits are comma separated, expressed in degrees, without spaces, negative for west longitudes and south latitudes" << std::endl;
	std::cout << "-o: optional, output file .kmz, .txt (OpenAir), .cup (SeeYou), .img (Garmin) or .mp (Polish). If not specified will be used the name of first input file as KMZ" << std::endl;
	std::cout << "-p: optional, when writing in OpenAir avoid to use arcs and circles but only points (DP)" << std::endl;
	std::cout << "-s: optional, when writing in OpenAir use coordinates with seconds (DD:MM:SS) instead of decimal minutes (DD:MM.MMM)" << std::endl;
	std::cout << "-t: optional, when reading KML/KMZ files treat also tracks as airspaces" << std::endl;
	std::cout << "-v: print version number" << std::endl;
	std::cout << "-h: print this guide" << std::endl << std::endl;
	std::cout << "At least one input airspace or waypoint file must be present." << std::endl;
	std::cout << "Warning: any already existing output file will be overwritten." << std::endl << std::endl;
}

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		printHelp();
		return EXIT_FAILURE;
	}

	AirspaceConverter ac;
	bool limitsAreSet(false);
	double topLat(90), bottomLat(-90), leftLon(-180), rightLon(180);
	std::string openAIPdir;

	for(int i=1; i<argc; i++) {
		size_t len=strlen(argv[i]);
		if(len != 2 || argv[i][0]!='-') {
			std::cerr << "Expected an option but found: " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}

		bool hasValueAfter = i+1 < argc && ((strlen(argv[i+1])>0 && argv[i+1][0]!='-') || (strlen(argv[i+1])>=2 && argv[i+1][0]=='-' && AirspaceConverter::isDigit(argv[i+1][1])));

		switch (argv[i][1]) {
		case 'q':
			if(!hasValueAfter) std::cerr << "ERROR: QNH value not found, using default value: " << ac.GetQNH() << " hPa."<< std::endl;
			else try {
				ac.SetQNH(std::stod(argv[++i]));
			} catch (...) {
				std::cerr << "ERROR: QNH value not valid, using default value: " << ac.GetQNH() << " hPa." << std::endl;
			}
			break;
		case 'a':
			if(!hasValueAfter) std::cerr << "ERROR: default altitude value not found, using default value: " << ac.GetDefaultTearrainAlt() << " m."<< std::endl;
			else try {
				ac.SetDefaultTearrainAlt(std::stod(argv[++i]));
			} catch (...) {
				std::cerr << "ERROR: default altitude value not valid, using default value: " << ac.GetDefaultTearrainAlt() << " m." << std::endl;
			}
			break;
		case 'i':
			if(!hasValueAfter) std::cerr << "ERROR: input airspace file path not found."<< std::endl;
			else ac.AddAirspaceFile(argv[++i]);
			break;
		case 'w':
			if(!hasValueAfter) std::cerr << "ERROR: input waypoint file path not found."<< std::endl;
			else ac.AddWaypointFile(argv[++i]);
			break;
		case 'm':
			if(!hasValueAfter) std::cerr << "ERROR: terrain map file path not found."<< std::endl;
			ac.AddTerrainRasterMapFile(argv[++i]);
			break;
		case 'o':
			if(!hasValueAfter) std::cerr << "ERROR: output file path not found."<< std::endl;
			else ac.SetOutputFile(argv[++i]);
			break;
		case 'l':
			if (!hasValueAfter) std::cerr << "ERROR: limit bounds not found." << std::endl;
			else {
				const std::string limits(argv[++i]);
				boost::tokenizer<boost::char_separator<char>> tokens(limits, boost::char_separator<char>(","));
				if (std::distance(tokens.begin(), tokens.end()) != 4) {
					std::cerr << "ERROR: wrong number (expected 4) of limit bounds found." << std::endl;
					break;
				}
				try {
					// Top Lat
					boost::tokenizer<boost::char_separator<char>>::iterator token = tokens.begin();
					if (!(*token).empty()) topLat = std::stod(*token);

					// Bottom Lat
					token++;
					if (!(*token).empty()) bottomLat = std::stod(*token);

					// Left Lon
					token++;
					if (!(*token).empty()) leftLon = std::stod(*token);

					// Right Lon
					token++;
					if (!(*token).empty()) rightLon = std::stod(*token);

					limitsAreSet = true;
				} catch (...) {
					std::cerr << "ERROR: unable to parse limit bounds." << std::endl;
				}
			}
			break;
		case 'p':
			ac.DoNotCalculateArcsAndCirconferences();
			break;
		case 's':
			ac.WriteCoordinatesAsDDMMSS();
			break;
		case 'h':
			printHelp();
			if (argc == 2) return EXIT_SUCCESS;
			break;
		case 't':
			ac.ProcessTracksAsAirspaces();
			break;
		case 'v':
			std::cout << "AirspaceConverter version: " << VERSION << std::endl;
			std::cout << "Compiled on " << __DATE__ << " at " << __TIME__ << std::endl;
			std::cout << "Copyright(C) 2016-2019 Alberto Realis-Luc" << std::endl;
			std::cout << "http://www.alus.it/AirspaceConverter" << std::endl << std::endl;
			if (argc == 2) return EXIT_SUCCESS;
			break;
		case 'd':
			if(hasValueAfter) openAIPdir = argv[++i];
			else std::cerr << "ERROR: input openAIP airspace directory path not found."<< std::endl;
			break;
		default:
			std::cerr << "Warning: Skipping unknown option: " << argv[i] << std::endl;
			break;
		}
	}

	if (openAIPdir.empty() && ac.GetNumberOfAirspaceFiles() == 0 && ac.GetNumberOfWaypointFiles() == 0) {
		std::cerr << "ERROR: No input files (airspace or waypoint) specified." << std::endl;
		return EXIT_FAILURE;
	}

	// If on Windows determine the proper paths
#ifdef _WIN32
	const std::string basePath(boost::filesystem::system_complete(boost::filesystem::path(argv[0])).parent_path().string());
	ac.SetIconsPath(basePath + "\\icons\\");
	ac.Set_cGPSmapperCommand('"' + basePath + "\\cGPSmapper\\cgpsmapper.exe\"");
#endif

	// Start the timer
	const auto startTime = std::chrono::high_resolution_clock::now();

	// Load raster maps
	ac.LoadTerrainRasterMaps();

	bool result(false);

	if (openAIPdir.empty()) {
		// Load airspaces and waypoints
		ac.LoadAirspaces();
		ac.LoadWaypoints();

		// Verify that there is at least one airspace or waypoint
		if(ac.GetNumOfAirspaces() == 0 && ac.GetNumOfWaypoints() == 0) {
			std::cerr << "ERROR: no usable data found in the input files specified." << std::endl;
			return EXIT_FAILURE;
		}

		// Apply filter if required
		if (limitsAreSet && !ac.FilterOnLatLonLimits(topLat, bottomLat, leftLon, rightLon)) std::cerr << "ERROR: filter limit bounds are not valid." << std::endl;

		// Convert!
		result = ac.Convert();

	} else result = ac.ConvertOpenAIPdir(openAIPdir);

	// Stop the timer
	const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
	std::cout<<"Total execution time: " << elapsedTimeSec << " sec." << std::endl << std::endl;

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

