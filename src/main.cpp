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
#include "OpenAIPreader.h"
#include "KMLwriter.h"
#include "PFMwriter.h"
#include "OpenAir.h"
#include <iostream>
#include <cstring>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>

void printHelp() {
	std::cout << "Example usage: AirspaceConverter -q 1013 -a 35 -i inputFileOpenAir.txt -i inputFileOpenAIP.aip -m terrainMap.dem -o outputFile.kmz" << std::endl << std::endl;
	std::cout << "Possible options:" << std::endl;
	std::cout << "-q: optional, specify the QNH in hPa used to calculate height of flight levels" << std::endl;
	std::cout << "-a: optional, specify a default terrain altitude in meters to calculate AGL heights of points not covered by loaded terrain map(s)" << std::endl;
	std::cout << "-i: mandatory, multiple, input file(s) can be OpenAir (.txt) or OpenAIP (.aip)" << std::endl;
	std::cout << "-m: optional, multiple, terrain map file(s) (.dem) used to lookup terrain height" << std::endl;
	std::cout << "-o: optional, output file .kml, .kmz, .mp (Polish) or .txt (OpenAir) if not specified will be used the name of first input file as KMZ" << std::endl;
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

	// Determine what kind of output is requested
	AirspaceConverter::OutputType outputType = AirspaceConverter::KMZ;

	// Prepare output filename if not entered by user
	if (!outputFile.empty()) {
		std::string outputExt(boost::filesystem::path(outputFile).extension().string());
		if (boost::iequals(outputExt,".kmz")) { /* already KMZ by default */ }
		else if(boost::iequals(outputExt,".kml")) outputType = AirspaceConverter::KML;
		else if(boost::iequals(outputExt,".mp")) outputType = AirspaceConverter::Polish;
		else if(boost::iequals(outputExt,".txt")) outputType = AirspaceConverter::OpenAir;
		else {
			std::cerr << "FATAL ERROR: Output file extension unknown." << std::endl;
			return EXIT_FAILURE;
		}
	}
	// Set QNH
	Altitude::SetQNH(QNH);

	// Set default terrain altitude
	KMLwriter::SetDefaultTerrainAltitude(defaultTerrainAlt);

	// Initialize airspaces multimap
	std::multimap<int, Airspace> airspaces;

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
				KMLwriter writer;
				flag = writer.WriteFile(outputFile, airspaces);
			}
			break;
		case AirspaceConverter::OpenAir:
			flag = openAir.WriteFile(outputFile);
			break;
		case AirspaceConverter::Polish:
			{
				// Make Polish file
				PFMwriter writer;
				flag = writer.WriteFile(outputFile, airspaces);
			}
			break;

		default:
			assert(false);
			break;
	}

	return flag ? EXIT_SUCCESS : EXIT_FAILURE;
}

