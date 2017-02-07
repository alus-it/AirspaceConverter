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

#pragma once

#ifndef VERSION
#define VERSION "0.1.9"
#endif

#include <functional>
#include <string>
#include <vector>
#include <map>
#include <istream>

class Airspace;
class Waypoint;

class AirspaceConverter {
public:
	enum OutputType {
		KMZ = 0,
		OpenAir_Format,
		Polish,
		Garmin,
		NumOfOutputTypes
	};

	AirspaceConverter();
	~AirspaceConverter();

	static std::function<void(const std::string&, const bool)> LogMessage;
	inline static void SetLogMessageFuntion(std::function<void(const std::string&, const bool)> func) { LogMessage = func; }
	static std::istream& SafeGetline(std::istream& is, std::string& line, bool& isCRLF);

	bool AddInputFile(const std::string& inputFile);
	inline void AddWaypointsFile(const std::string& waypointsFile) { CUPfiles.push_back(waypointsFile); }
	inline void AddTerrainRasterMapFile(const std::string& rasterMapFile) { DEMfiles.push_back(rasterMapFile); }
	void LoadAirspaces();
	void LoadTerrainRasterMaps();
	void UnloadAirspaces();
	void UnloadRasterMaps();
	void LoadWaypoints();
	void UnloadWaypoints();
	void SetQNH(const double newQNHhPa);
	double GetQNH() const;
	void SetDefaultTearrainAlt(const double altMt);
	double GetDefaultTearrainAlt() const;
	bool Convert(const std::string& outputFilename, const AirspaceConverter::OutputType type);
	inline bool ConversionDone() const { return conversionDone; }
	inline bool WereAllAGLaltitudesCovered() const { return allAGLaltitudesCovered; }
	inline std::string GetOutputFilename() const { return outputFile; }
	inline unsigned long GetNumOfAirspaces() const { return (unsigned long)airspaces.size(); }
	inline unsigned long GetNumOfWaypoints() const { return (unsigned long)waypoints.size(); }
	int GetNumOfTerrainMaps() const;
	
	static const std::vector<std::string> disclaimer;

private:
	static void DefaultLogMessage(const std::string&, const bool isError = false);

	std::multimap<int, Airspace> airspaces;
	std::multimap<int, Waypoint*> waypoints;
	std::string outputFile;
	std::vector<std::string> openAIPinputFiles, openAirInputFiles, DEMfiles, CUPfiles;
	bool conversionDone;
	bool allAGLaltitudesCovered;
};
