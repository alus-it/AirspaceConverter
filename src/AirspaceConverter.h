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
#define VERSION "0.2.0"
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
		KMZ_Format = 0,
		OpenAir_Format,
		Polish_Format,
		Garmin_Format,
		Unknown_Format
	};

	AirspaceConverter();
	~AirspaceConverter();

	static std::function<void(const std::string&, const bool)> LogMessage;
	static std::function<bool(const std::string&, const std::string&)> cGPSmapper;

	inline static void SetLogMessageFunction(std::function<void(const std::string&, const bool)> func) { LogMessage = func; }
	inline static void Set_cGPSmapperFunction(std::function<bool(const std::string&, const std::string&)> func) { cGPSmapper = func; }
	inline static void Set_cGPSmapperCommand(const std::string& cGPSmapperCmd) { cGPSmapperCommand = cGPSmapperCmd; }
	static void SetIconsPath(const std::string& iconsPath);
	static std::istream& SafeGetline(std::istream& is, std::string& line, bool& isCRLF);
	static OutputType DetermineType(const std::string& filename);
	static bool PutTypeExtension(const OutputType type, std::string& filename);
	static bool ParseAltitude(const std::string& text, const bool isTop, Airspace& airspace);
	inline static bool isDigit(const char c) { return (c >= '0' && c <= '9'); }

	inline void AddAirspaceFile(const std::string& inputFile) { airspaceFiles.push_back(inputFile); }
	inline void AddWaypointFile(const std::string& waypointsFile) { waypointFiles.push_back(waypointsFile); }
	inline void AddTerrainRasterMapFile(const std::string& rasterMapFile) { terrainRasterMapFiles.push_back(rasterMapFile); }
	inline int GetNumberOfAirspaceFiles() const { return (int)airspaceFiles.size(); }
	inline int GetNumberOfWaypointFiles() const { return (int)waypointFiles.size(); }
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
	bool Convert();
	inline bool IsConversionDone() const { return conversionDone; }
	inline OutputType GetOutputType() const { return DetermineType(outputFile); }
	inline bool SetOutputType(const OutputType type) { return PutTypeExtension(type, outputFile); }
	inline void SetOutputFile(const std::string& outputFilename) { outputFile = outputFilename; }
	inline std::string GetOutputFile() const { return outputFile; }
	inline unsigned long GetNumOfAirspaces() const { return (unsigned long)airspaces.size(); }
	inline unsigned long GetNumOfWaypoints() const { return (unsigned long)waypoints.size(); }
	int GetNumOfTerrainMaps() const;
	
	static const std::vector<std::string> disclaimer;

private:
	static void DefaultLogMessage(const std::string&, const bool isError = false);
	static bool Default_cGPSmapper(const std::string& polishFile, const std::string& outputFile);

	static std::string cGPSmapperCommand;
	std::multimap<int, Airspace> airspaces;
	std::multimap<int, Waypoint> waypoints;
	std::string outputFile;
	std::vector<std::string> airspaceFiles, terrainRasterMapFiles, waypointFiles;
	bool conversionDone;
};
