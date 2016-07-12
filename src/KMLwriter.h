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

#include <string>
#include <vector>
#include <map>
#include <fstream>

class Altitude;
class Airspace;
class RasterMap;

class KMLwriter
{
public:
	KMLwriter();
	~KMLwriter();

	bool WriteFile(const std::string& filename, const std::multimap<int, Airspace>& airspaces);
	static bool AddTerrainMap(const std::string& filename);
	inline static int GetNumOfRasterMaps() { return terrainMaps.size(); }
	static void ClearTerrainMaps();
	inline static void SetDefaultTerrainAltitude(const double& defaultAltMt) { defaultTerrainAltitudeMt = defaultAltMt; }
	inline static double GetDefaultTerrainAltitude() { return defaultTerrainAltitudeMt; }
	inline bool WereAllAGLaltitudesCovered() const { return allAGLaltitudesCovered; }
	bool CompressToKMZ(const std::string& inputKMLfile, const bool deleteOriginal = true);

private:
	void WriteHeader();
	void OpenPlacemark(const Airspace& airspace);
	void OpenPolygon(const bool extrude, const bool absolute);
	void ClosePolygon();
	void WriteSideWalls(const Airspace& airspace);
	void WriteSideWalls(const Airspace& airspace, const std::vector<double>& altitudesAmsl);
	void WriteBaseOrTop(const Airspace& airspace, const Altitude& alt, const bool extrudeToGround = false);
	void WriteBaseOrTop(const Airspace& airspace, const std::vector<double>& altitudesAmsl);

	static bool GetTerrainAltitudeMt(const double& lat, const double& lon, double&alt);
	
	static const std::string disclaimer;
	static const std::string colors[][2];
	static std::vector<RasterMap*> terrainMaps;
	static double defaultTerrainAltitudeMt;
	
	std::ofstream file;
	bool allAGLaltitudesCovered;
};

