//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2017 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "AirspaceConverter.h"
#include <iostream>
#include <cstring>
#include <chrono>

void printHelp() {
	std::cout << "Example usage: AirspaceConverter -q 1013 -a 35 -i inputFileOpenAir.txt -i inputFileOpenAIP.aip -m terrainMap.dem -o outputFile.kmz" << std::endl << std::endl;
	std::cout << "Possible options:" << std::endl;
	std::cout << "-q: optional, specify the QNH in hPa used to calculate height of flight levels" << std::endl;
	std::cout << "-a: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)" << std::endl;
	std::cout << "-i: mandatory, multiple, input file(s) can be OpenAir (.txt), OpenAIP (.aip) or CUP waypoints (.cup)" << std::endl;
	std::cout << "-m: optional, multiple, terrain map file(s) (.dem) used to lookup terrain height" << std::endl;
	std::cout << "-o: optional, output file .kmz, .txt (OpenAir), .img (Garmin) or .mp (Polish). If not specified will be used the name of first input file as KMZ" << std::endl;
	std::cout << "-v: print version number" << std::endl;
	std::cout << "-h: print this guide" << std::endl << std::endl;
}

int main(int argc, char *argv[]) {
	if (argc <= 1)
	{
		printHelp();
		return EXIT_FAILURE;
	}

	AirspaceConverter ac;

	for(int i=1; i<argc; i++) {
		int len=strlen(argv[i]);
		if(len != 2 || argv[i][0]!='-') {
			std::cerr << "Expected an option but found: " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}

		bool hasValueAfter = i+1 < argc && strlen(argv[i+1])>0 && argv[i+1][0]!='-';

		switch (argv[i][1]) {
		case 'q':
			if(!hasValueAfter) std::cerr << "ERROR: QNH value not found, using default value: " << ac.GetQNH() << " hPa."<< std::endl;
			else ac.SetQNH(std::stod(argv[++i]));
			break;
		case 'a':
			if(!hasValueAfter) std::cerr << "ERROR: default altitude value not found, using default value: " << ac.GetDefaultTearrainAlt() << " m."<< std::endl;
			else ac.SetDefaultTearrainAlt(std::stod(argv[++i]));
			break;
		case 'i':
			if(!hasValueAfter) std::cerr << "ERROR: input file path not found."<< std::endl;
			else ac.AddAirspaceFile(argv[++i]);
			break;
		case 'm':
			if(!hasValueAfter) std::cerr << "ERROR: terrain map file path not found."<< std::endl;
			ac.AddTerrainRasterMapFile(argv[++i]);
			break;
		case 'o':
			if(!hasValueAfter) std::cerr << "ERROR: output file path not found."<< std::endl;
			else ac.SetOutputFile(argv[++i]);
			break;
		case 'h':
			printHelp();
			if (argc == 2) return EXIT_SUCCESS;
			break;
		case 'v':
			std::cout << "AirspaceConverter version: " << VERSION << std::endl;
			std::cout << "Copyright(C) 2016-2017 Alberto Realis-Luc" << std::endl;
			std::cout << "http://www.alus.it/AirspaceConverter" << std::endl << std::endl;
			if (argc == 2) return EXIT_SUCCESS;
			break;
		default:
			std::cerr << "Warning: Skipping unknown option: " << argv[i] << std::endl;
			break;
		}
	}

	if (ac.GetNumberOfAirspaceFiles() == 0) {
		std::cerr << "FATAL ERROR: No input files specified." << std::endl;
		return EXIT_FAILURE;
	}

	// Start the timer
	const auto startTime = std::chrono::high_resolution_clock::now();

	// Process input files
	ac.LoadAirspaces();
	ac.LoadWaypoints();
	ac.LoadTerrainRasterMaps();

	if(ac.GetNumOfAirspaces() == 0 && ac.GetNumOfWaypoints() == 0) {
		std::cerr << "FATAL ERROR: no input files loaded." << std::endl;
		return EXIT_FAILURE;
	}
	
	// Convert!
	bool flag = ac.Convert();

	// Stop the timer
	const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
	std::cout<<"Total execution time: " << elapsedTimeSec << " sec." << std::endl << std::endl;

	return flag ? EXIT_SUCCESS : EXIT_FAILURE;
}

