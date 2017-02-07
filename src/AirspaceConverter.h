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
#include <thread>

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
	static std::istream& safeGetline(std::istream& is, std::string& line, bool& isCRLF);

	bool AddInputFile(const std::string& inputFile);
	inline int GetNumberOfInputFiles() const { return (int)openAIPinputFiles.size() + (int)openAirInputFiles.size(); }
	inline void AddWaypointsFile(const std::string& waypointsFile) { CUPfiles.push_back(waypointsFile); }
	inline void AddRasterMap(const std::string& rasterMapFile) { DEMfiles.push_back(rasterMapFile); }
	bool LoadAirspacesFiles();
	bool LoadDEMfiles();
	bool UnloadAirspaces();
	bool UnloadRasterMaps();
	bool LoadWaypointsFiles();
	void LoadWaypointsFilesThread();
	bool UnloadWaypoints();
	bool MakeKMZfile(const std::string& outputKMZfile, const double& defaultTerraninAltMt);
	bool MakeOtherFile(const std::string& outputFilename, const AirspaceConverter::OutputType type);
	inline unsigned long GetNumOfAirspaces() const { return (unsigned long)airspaces.size(); }
	inline unsigned long GetNumOfWaypoints() const { return (unsigned long)waypoints.size(); }
	int GetNumOfTerrainMaps() const;
	inline void Join() { if (workerThread.joinable()) workerThread.join(); }

	static const std::vector<std::string> disclaimer;

private:
	static void DefaultLogMessage(const std::string&, const bool isError = false);

	void LoadAirspacesfilesThread();
	void LoadDEMfilesThread();
	void MakeKMZfileThread();
	void MakeOtherFileThread();

	std::thread workerThread;
	std::multimap<int, Airspace> airspaces;
	std::multimap<int, Waypoint*> waypoints;
	OutputType outputType;
	std::string outputFile;
	std::vector<std::string> openAIPinputFiles, openAirInputFiles, DEMfiles, CUPfiles;
};
