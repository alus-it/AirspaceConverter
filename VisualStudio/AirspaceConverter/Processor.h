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

#pragma once
#include "../../src/AirspaceConverter.h"
#include <thread>
//#include <deque>
#include <map>
#include <string>
#include <vector>

#define WM_GENERAL_WORK_DONE		WM_USER+1
#define WM_WRITE_OUTPUT_OK			WM_USER+2
#define WM_WRITE_KML_AGL_WARNING	WM_USER+3

class Airspace;
class Waypoint;

class Processor {
public:
	Processor(HWND hwnd);
	~Processor();
	bool AddInputFile(const std::string& inputFile);
	inline void AddWaypointsFile(const std::string& waypointsFile) { CUPfiles.push_back(waypointsFile); }
	inline void AddRasterMap(const std::string& rasterMapFile) { DEMfiles.push_back(rasterMapFile); }
	bool LoadAirspacesFiles(const double& QNH);
	bool LoadDEMfiles();
	bool UnloadAirspaces();
	bool UnloadRasterMaps();
	bool LoadWaypointsFiles();
	void LoadWaypointsFilesThread();
	bool UnloadWaypoints();
	bool MakeKMLfile(const std::string& outputKMLfile, const double& defaultTerraninAltMt);
	bool MakeOtherFile(const std::string& outputFilename, const AirspaceConverter::OutputType type);
	inline void SetWindow(HWND hwnd) { window = hwnd; }
	inline unsigned long GetNumOfAirspaces() const { return airspaces.size(); }
	inline unsigned long GetNumOfWaypoints() const { return waypoints.size(); }
	int GetNumOfTerrainMaps() const;
	inline void Join() { if (workerThread.joinable()) workerThread.join(); }
	//inline void Abort() { abort = true; Join(); }
	
private:
	void LoadAirspacesfilesThread();
	void LoadDEMfilesThread();
	void MakeKMLfileThread();
	void MakeOtherFileThread();

	HWND window;
	std::thread workerThread;
	//std::deque<std::string> queue;
	//bool abort;
	std::multimap<int, Airspace> airspaces;
	std::multimap<int, Waypoint*> waypoints;
	double QNH;
	double defaultTerrainAlt;
	std::string outputFile;
	std::vector<std::string> openAIPinputFiles, openAirInputFiles, DEMfiles, CUPfiles;
	AirspaceConverter::OutputType outputType;
};
