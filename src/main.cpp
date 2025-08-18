//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "AirspaceConverter.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <boost/tokenizer.hpp>
#include "Altitude.hpp"

void printHelp() {
	std::cout << "Example usage: airspaceconverter -q 1013 -a 35 -i inputFileOpenAir.txt -i openAIP_asp.aip -w waypoints.cup -w openAIP_wpt.aip -m terrainMap.dem -o outputFile.kmz" << std::endl << std::endl;
	std::cout << "Possible options:" << std::endl;
	std::cout << "-q: optional, specify the QNH in hPa used to calculate height of flight levels" << std::endl;
	std::cout << "-a: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)" << std::endl;
	std::cout << "-i: multiple, input airspace file(s) can be OpenAir (.txt), openAIP (.aip), Google Earth (.kmz, .kml)" << std::endl;
	std::cout << "-w: multiple, input waypoint file(s) can be SeeYou (.cup), LittleNavMap (.csv) or openAIP (.aip)" << std::endl;
	std::cout << "-m: optional, multiple, terrain map file(s) (.dem) used to lookup terrain heights" << std::endl;
	std::cout << "-l: optional, set filter limits in latitude, longitude for the output, followed by the 4 limit values: northLat,southLat,westLon,eastLon" << std::endl;
	std::cout << "    where the limits are comma separated, coordinates expressed in degrees, without spaces, negative for west longitudes and south latitudes" << std::endl;
	std::cout << "-u: optional, set filter limits on altitude for the output, followed by the 1 or 2 limit values: lowAlt,hiAlt" << std::endl;
	std::cout << "    altitudes are expressed in feet, at main sea level. If higher limit is omitted it will be considered as unlimited" << std::endl;
	std::cout << "-o: optional, output file .kmz, .txt (OpenAir), .cup (SeeYou), .csv (LittleNavMap)";
	if (AirspaceConverter::Is_cGPSmapperAvailable()) std::cout << ", .img (Garmin)";
	std::cout << " or .mp (Polish). If not specified will be used the name of first input file as KMZ" << std::endl;
	std::cout << "-p: optional, when writing in OpenAir avoid to use arcs and circles but only points (DP)" << std::endl;
	std::cout << "-s: optional, when writing in OpenAir use coordinates always with seconds (DD:MM:SS)" << std::endl;
	std::cout << "-d: optional, when writing in OpenAir use coordinates always with decimal minutes (DD:MM.MMM)" << std::endl;
	std::cout << "-t: optional, when reading KML/KMZ files treat also tracks as airspaces" << std::endl;
	std::cout << "-v: print version number" << std::endl;
	std::cout << "-h: print this guide" << std::endl << std::endl;
	std::cout << "At least one input airspace or waypoint file must be present." << std::endl;
	std::cout << "Warning: any already existing output file will be overwritten." << std::endl;
	if (!AirspaceConverter::Is_cGPSmapperAvailable()) std::cout << "Warning: cGPSmapper not found so the conversion to Garmin IMG is not possible." << std::endl;
	std::cout << std::endl;
}

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		printHelp();
		return EXIT_FAILURE;
	}

	AirspaceConverter ac;
	bool positionLimitsAreSet(false), altitudeLimitsAreSet(false);
	double topLat(90), bottomLat(-90), leftLon(-180), rightLon(180);
	Altitude limitLowAltitude(-10000), limitHiAltitude;
	limitHiAltitude.SetUnlimited();

	for(int i=1; i<argc; i++) {
		size_t len=strlen(argv[i]);
		if(len != 2 || argv[i][0]!='-') {
			std::cerr << "Expected an option but found: " << argv[i] << std::endl << std::endl;
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
			if(!hasValueAfter) std::cerr << "ERROR: default altitude value not found, using default value: " << AirspaceConverter::GetDefaultTerrainAlt() << " m."<< std::endl;
			else try {
				AirspaceConverter::SetDefaultTerrainAlt(std::stod(argv[++i]));
			} catch (...) {
				std::cerr << "ERROR: default altitude value not valid, using default value: " << AirspaceConverter::GetDefaultTerrainAlt() << " m." << std::endl;
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
			if (!hasValueAfter) std::cerr << "ERROR: filter on position limit bounds not found." << std::endl;
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

					positionLimitsAreSet = true;
				} catch (...) {
					std::cerr << "ERROR: unable to parse limit bounds." << std::endl;
				}
			}
			break;
		case 'u':
			if (!hasValueAfter) std::cerr << "ERROR: altitude limits not found." << std::endl;
			else {
				const std::string limits(argv[++i]);
				boost::tokenizer<boost::char_separator<char>> tokens(limits, boost::char_separator<char>(","));
				const int numOfTokens = std::distance(tokens.begin(), tokens.end());
				if (numOfTokens < 1 || numOfTokens > 2) {
					std::cerr << "ERROR: wrong number (expected 1 or 2) of limit found." << std::endl;
					break;
				}
				try {
					// Low altitude
					boost::tokenizer<boost::char_separator<char>>::iterator token = tokens.begin();
					if (!(*token).empty()) limitLowAltitude = Altitude(std::stod(*token));

					// Hi altitude
					if (numOfTokens > 1) {
						token++;
						if (!(*token).empty()) limitHiAltitude = Altitude(std::stod(*token));
					}

					altitudeLimitsAreSet = true;
				} catch (...) {
					std::cerr << "ERROR: unable to parse limit bounds." << std::endl;
				}
			}
			break;
		case 'p':
			ac.DoNotCalculateArcsAndCirconferences();
			break;
		case 's':
			ac.SetOpenAirCoodinatesInSeconds();
			break;
		case 'd':
			ac.SetOpenAirCoodinatesInDecimalMinutes();
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
			{
				int diffLatestVersion;
				if (AirspaceConverter::CheckForNewVersion(diffLatestVersion)) {
					if (diffLatestVersion == 0) std::cout << "This is the latest version." << std::endl;
					else if (diffLatestVersion > 0) std::cout << "A new version is available!" << std::endl;
					else std::cout << "This version is not yet released." << std::endl;
				}
			}
			std::cout << "Copyright(C) 2016 Alberto Realis-Luc, Valerio Messina" << std::endl;
			std::cout << "https://www.alus.it/AirspaceConverter" << std::endl << std::endl;
			if (argc == 2) return EXIT_SUCCESS;
			break;
		default:
			std::cerr << "Warning: Skipping unknown option: " << argv[i] << std::endl;
			break;
		}
	}

	if (ac.GetNumberOfAirspaceFiles() == 0 && ac.GetNumberOfWaypointFiles() == 0) {
		std::cerr << "ERROR: No input files (airspace or waypoint) specified." << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	// Start the timer
	const auto startTime = std::chrono::high_resolution_clock::now();

	// Load raster maps (terrain maps must be loaded before waypoints if we want to use them to correct altitudes)
	ac.LoadTerrainRasterMaps();

	bool result(false);

	// Load airspaces and waypoints
	ac.LoadAirspaces();
	ac.LoadWaypoints();

	// Verify that there is at least one airspace or waypoint
	if(ac.GetNumOfAirspaces() == 0 && ac.GetNumOfWaypoints() == 0) {
		std::cerr << "ERROR: no usable data found in the input files specified." << std::endl << std::endl;
		return EXIT_FAILURE;
	}

	// Apply altitude filter if required
	if (altitudeLimitsAreSet && !ac.FilterOnAltitudeLimits(limitLowAltitude, limitHiAltitude)) std::cerr << "ERROR: filter altitude limit are not valid." << std::endl;

	// Apply position filter if required
	if (positionLimitsAreSet && !ac.FilterOnLatLonLimits(topLat, bottomLat, leftLon, rightLon)) std::cerr << "ERROR: filter position limit bounds are not valid." << std::endl;

	// Convert!
	result = ac.Convert();

	// Stop the timer
	const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
	std::cout << "Total execution time: " << elapsedTimeSec << " sec." << std::endl << std::endl;

	// The End
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
