//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2020 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once

#ifndef VERSION
#define VERSION "0.3.6"
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
		SeeYou_Format,
		Polish_Format,
		Garmin_Format,
		Unknown_Format
	};

	AirspaceConverter();
	~AirspaceConverter();

	static std::function<void(const std::string&)> LogMessage;
	static std::function<void(const std::string&)> LogWarning;
	static std::function<void(const std::string&)> LogError;
	static std::function<bool(const std::string&, const std::string&)> cGPSmapper;

	inline static void SetLogMessageFunction(std::function<void(const std::string&)> func) { LogMessage = func; }
	inline static void SetLogWarningFunction(std::function<void(const std::string&)> func) { LogWarning = func; }
	inline static void SetLogErrorFunction(std::function<void(const std::string&)> func) { LogError = func; }
	inline static void Set_cGPSmapperFunction(std::function<bool(const std::string&, const std::string&)> func) { cGPSmapper = func; }
	inline static bool Is_cGPSmapperAvailable() { return !cGPSmapperCommand.empty(); }
	static double FrequencyMHz(const int& frequencyHz) { return 0.000001 * frequencyHz; }
	static double FrequencykHz(const int& frequencyHz) { return 0.001 * frequencyHz; }
	static std::istream& SafeGetline(std::istream& is, std::string& line, bool& isCRLF);
	static OutputType DetermineType(const std::string& filename);
	static bool PutTypeExtension(const OutputType type, std::string& filename);
	static bool ParseAltitude(const std::string& text, const bool isTop, Airspace& airspace);
	inline static bool isDigit(const char c) { return (c >= '0' && c <= '9'); }
	static std::string GetCreationDateString();
	static bool CheckAirbandFrequency(const double& frequencyMHz, int& frequencyHz);
	static bool CheckVORfrequency(const double& frequencyMHz, int& frequencyHz);
	static bool CheckNDBfrequency(const double& frequencykHz, int& frequencyHz);
	inline void AddAirspaceFile(const std::string& inputFile) { airspaceFiles.push_back(inputFile); }
	inline void AddWaypointFile(const std::string& waypointsFile) { waypointFiles.push_back(waypointsFile); }
	inline void AddTerrainRasterMapFile(const std::string& rasterMapFile) { terrainRasterMapFiles.push_back(rasterMapFile); }
	inline int GetNumberOfAirspaceFiles() const { return (int)airspaceFiles.size(); }
	inline int GetNumberOfWaypointFiles() const { return (int)waypointFiles.size(); }
	void LoadAirspaces(const OutputType suggestedTypeForOutputFilename = OutputType::KMZ_Format);
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
	bool ConvertOpenAIPdir(const std::string openAIPdir);
	inline bool IsConversionDone() const { return conversionDone; }
	inline OutputType GetOutputType() const { return DetermineType(outputFile); }
	inline bool SetOutputType(const OutputType type) { return PutTypeExtension(type, outputFile); }
	inline void SetOutputFile(const std::string& outputFilename) { outputFile = outputFilename; }
	inline std::string GetOutputFile() const { return outputFile; }
	inline unsigned long GetNumOfAirspaces() const { return (unsigned long)airspaces.size(); }
	inline unsigned long GetNumOfWaypoints() const { return (unsigned long)waypoints.size(); }
	int GetNumOfTerrainMaps() const;
	bool FilterOnLatLonLimits(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon);
	inline void ProcessTracksAsAirspaces(const bool treatTracksAsAirspaces = true) { processLineStrings = treatTracksAsAirspaces; }
	static void DoNotCalculateArcsAndCirconferences(const bool doNotCalcArcs = true);
	static void SetOpenAirCoodinatesAutomatic();
	static void SetOpenAirCoodinatesInDecimalMinutes();
	static void SetOpenAirCoodinatesInSeconds();

	static const std::vector<std::string> disclaimer;
	static const std::string basePath;
	static const std::string cGPSmapperCommand;

private:
	static void DefaultLogMessage(const std::string& text);
	static void DefaultLogWarning(const std::string& text);
	static void DefaultLogError(const std::string& text);
	static bool Default_cGPSmapper(const std::string& polishFile, const std::string& outputFile);
	static const std::string Detect_cGPSmapperPath();

	std::multimap<int, Airspace> airspaces;
	std::multimap<int, Waypoint*> waypoints;
	std::string outputFile;
	std::vector<std::string> airspaceFiles, terrainRasterMapFiles, waypointFiles;
	bool conversionDone;
	bool processLineStrings;
};
