//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "AirspaceConverter.h"
#include "Airspace.h"
#include "CUPreader.h"
#include "OpenAIPreader.h"
#include "KMLwriter.h"
#include "PFMwriter.h"
#include "OpenAir.h"
#include "Waypoint.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

void printHelp() {
	std::cout << "Example usage: AirspaceConverter -q 1013 -a 35 -i inputFileOpenAir.txt -i inputFileOpenAIP.aip -m terrainMap.dem -o outputFile.kmz" << std::endl << std::endl;
	std::cout << "Possible options:" << std::endl;
	std::cout << "-q: optional, specify the QNH in hPa used to calculate height of flight levels" << std::endl;
	std::cout << "-a: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)" << std::endl;
	std::cout << "-i: mandatory, multiple, input file(s) can be OpenAir (.txt), OpenAIP (.aip) or CUP waypoints (.cup)" << std::endl;
	std::cout << "-m: optional, multiple, terrain map file(s) (.dem) used to lookup terrain height" << std::endl;
	std::cout << "-o: optional, output file .kmz, .kml, .txt (OpenAir), .img (Garmin) or .mp (Polish). If not specified will be used the name of first input file as KMZ" << std::endl;
	std::cout << "-v: print version number" << std::endl;
	std::cout << "-h: print this guide" << std::endl << std::endl;
}

int main(int argc, char *argv[]) {
	if (argc <= 1)
	{
		printHelp();
		return EXIT_FAILURE;
	}

	double QNH = Altitude::GetQNH();
	double defaultTerrainAlt = KMLwriter::GetDefaultTerrainAltitude();
	std::string outputFile;
	std::vector<std::string> inputFiles, mapFiles;

	for(int i=1; i<argc; i++) {
		int len=strlen(argv[i]);
		if(len != 2 || argv[i][0]!='-') {
			std::cerr << "Expected an option but found: " << argv[i] << std::endl;
			return EXIT_FAILURE;
		}

		bool hasValueAfter = i+1 < argc && strlen(argv[i+1])>0 && argv[i+1][0]!='-';

		switch (argv[i][1]) {
		case 'q':
			if(!hasValueAfter) {
				std::cerr << "ERROR: QNH value not found, using default value: " << QNH << " hPa."<< std::endl;
				continue;
			}
			QNH = std::stod(argv[++i]);
			break;
		case 'a':
			if(!hasValueAfter) {
				std::cerr << "ERROR: default altitude value not found, using default value: " << defaultTerrainAlt << " m."<< std::endl;
				continue;
			}
			defaultTerrainAlt = std::stod(argv[++i]);
			break;
		case 'i':
			if(!hasValueAfter) {
				std::cerr << "ERROR: input file path not found."<< std::endl;
				continue;
			}
			inputFiles.push_back(argv[++i]);
			break;
		case 'm':
			if(!hasValueAfter) {
				std::cerr << "ERROR: terrain map file path not found."<< std::endl;
				continue;
			}
			mapFiles.push_back(argv[++i]);
			break;
		case 'o':
			if(!hasValueAfter) {
				std::cerr << "ERROR: output file path not found."<< std::endl;
				continue;
			}
			outputFile = argv[++i];
			break;
		case 'h':
			printHelp();
			if (argc == 2) return EXIT_SUCCESS;
			break;
		case 'v':
			std::cout << "AirspaceConverter version: " << VERSION << std::endl;
			std::cout << "Copyright(C) 2016 Alberto Realis-Luc" << std::endl;
			std::cout << "http://www.alus.it/AirspaceConverter" << std::endl << std::endl;
			if (argc == 2) return EXIT_SUCCESS;
			break;
		default:
			std::cerr << "Warning: Skipping unknown option: " << argv[i] << std::endl;
			break;
		}
	}

	if (inputFiles.empty()) {
		std::cerr << "FATAL ERROR: No input files specified." << std::endl;
		return EXIT_FAILURE;
	}

	// Start the timer
	const auto startTime = std::chrono::high_resolution_clock::now();

	// Determine what kind of output is requested
	AirspaceConverter::OutputType outputType = AirspaceConverter::KMZ;

	// Prepare output filename if not entered by user
	if (!outputFile.empty()) {
		std::string outputExt(boost::filesystem::path(outputFile).extension().string());
		//if (boost::iequals(outputExt, ".kmz")) { /* already KMZ by default */ }
		/*else*/ if(boost::iequals(outputExt, ".kml")) outputType = AirspaceConverter::KML;
		else if(boost::iequals(outputExt, ".mp")) outputType = AirspaceConverter::Polish;
		else if(boost::iequals(outputExt, ".txt")) outputType = AirspaceConverter::OpenAir;
		else if(boost::iequals(outputExt, ".img")) outputType = AirspaceConverter::Garmin;
		else {
			std::cerr << "FATAL ERROR: Output file extension unknown." << std::endl;
			return EXIT_FAILURE;
		}
	} else outputFile = boost::filesystem::path(inputFiles.front()).replace_extension(".kmz").string(); // Default output as KMZ

	// Set QNH
	Altitude::SetQNH(QNH);

	// Set default terrain altitude
	KMLwriter::SetDefaultTerrainAltitude(defaultTerrainAlt);

	// Initialize airspaces and waypoints multimaps
	std::multimap<int, Airspace> airspaces;
	std::multimap<int, Waypoint*> waypoints;

	// Initialize OpenAir module
	OpenAir openAir(airspaces);

	// Process input files
	bool flag = true; // all failed
	for(const std::string& inputFile : inputFiles) {
		std::string ext(boost::filesystem::path(inputFile).extension().string());
		if(boost::iequals(ext, ".txt")) {
			if(openAir.ReadFile(inputFile)) flag = false;
		} else if(boost::iequals(ext, ".aip")) {
			if(OpenAIPreader::ReadFile(inputFile, airspaces)) flag = false;
		} else if(boost::iequals(ext, ".cup")) {
			if(CUPreader::ReadFile(inputFile, waypoints)) flag = false;
		} else std::cerr << "ERROR: unknown extension: " << ext << "for input file" << std::endl;
	}
	if(flag) {
		std::cerr << "FATAL ERROR: no input files loaded." << std::endl;
		return EXIT_FAILURE;
	}
	
	switch(outputType) {
		case AirspaceConverter::KMZ:
		case AirspaceConverter::KML:
			{
				// Load terrain maps
				flag = true; // all failed
				for(const std::string& mapFile : mapFiles) if(KMLwriter::AddTerrainMap(mapFile)) flag = false;
				if(flag) std::cout << "Warning: no terrain map loaded, using default terrain height for all applicable AGL points." << std::endl;

				// Make KML file
				flag = KMLwriter().WriteFile(outputFile, airspaces, waypoints);
			}
			break;
		case AirspaceConverter::OpenAir:
			flag = openAir.WriteFile(outputFile);
			break;
		case AirspaceConverter::Polish:
			flag = PFMwriter().WriteFile(outputFile, airspaces);
			break;
		case AirspaceConverter::Garmin:
			{
				flag = false;

				// First make Polish file
				const std::string polishFile(boost::filesystem::path(outputFile).replace_extension(".mp").string());
				if(!PFMwriter().WriteFile(polishFile, airspaces)) break;

				// Then call cGPSmapper
				std::cout << "Invoking cGPSmapper to make: " << outputFile << std::endl << std::endl;

				//TODO: add arguments to create files also for other software like Garmin BaseCamp
				const std::string cmd(boost::str(boost::format("./cGPSmapper/cgpsmapper-static %1s -o %2s") %polishFile %outputFile));

				if(system(cmd.c_str()) == EXIT_SUCCESS) {
					flag = true;
					std::remove(polishFile.c_str()); // Delete polish file
				} else std::cerr << std::endl << "ERROR: cGPSmapper returned an error." << std::endl;
			}
			break;
		default:
			assert(false);
			break;
	}

	// Clear waypoints
	for (const std::pair<const int, Waypoint*>& wpt : waypoints) delete wpt.second;
	waypoints.clear();

	// Stop the timer
	const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
	std::cout<<"Total execution time: " << elapsedTimeSec << " sec." << std::endl << std::endl;

	return flag ? EXIT_SUCCESS : EXIT_FAILURE;
}

