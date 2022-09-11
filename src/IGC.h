//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2021 Valerio Messina, Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include <fstream>
//#include "Airspace.h"
//#include "Geometry.h"
#include "Waypoint.h"

class Waypoint;

class IGC {
friend class Point;
//friend class Circle;
//friend class Sector;

public:
	/*enum CoordinateType {
		DEG_DECIMAL_MIN = 0,
		DEG_MIN_SEC,
		AUTO
	};*/

	IGC(std::multimap<int,Waypoint*>& waypointsMap);
	~IGC() {}
	bool Read(const std::string& fileName);
	//bool Write(const std::string& fileName);
	//inline static void CalculateArcsAndCirconferences(const bool calcArcs = true) { calculateArcs = calcArcs; }
	//inline static void SetCoordinateType(CoordinateType type) { coordinateType = type; }

private:
	static std::string& RemoveComments(std::string &s);
	static bool ParseDegrees(const std::string& dddmmss, double& deg);
	static bool ParseCoordinates(const std::string& text, Geometry::LatLon& point);
	static bool ParseAN(const std::string& line, Waypoint& waypoint, const bool isUTF8 = false);
	static bool ParseAF(const std::string& line, Waypoint& waypoint, const bool isUTF8 = false);
	static bool ParseAltitude(const std::string& line, const bool isTop, Waypoint& waypoint);
	static bool ParseS (const std::string& line);
	static bool ParseT (const std::string& line);
	static bool ParseDP(const std::string& line, Waypoint& waypoint, const int& linenumber);
	static bool IsFileUTF8 (std::ifstream& inputFile);
	bool ParseAC(const std::string& line, Waypoint& waypoint);
	bool ParseV(const std::string& line, Waypoint& waypoint);
	bool ParseDA(const std::string& line, Waypoint& waypoint);
	bool ParseDB(const std::string& line, Waypoint& waypoint);
	bool ParseDC(const std::string& line, Waypoint& waypoint);
	//bool ParseDY(const std::string& line, Waypoint& waypoint); // Airway not yet supported
   bool ParseB(const std::string& line, Waypoint& waypoint);

	bool InsertWaypoint(Waypoint& waypoint);
	void WriteHeader();
	bool WriteCategory(const Waypoint& airsapce);
	void WritePoint(const Geometry::LatLon& point, bool isCenterPoint = false, bool addPrefix = true);
	void WritePoint(const Point& point);
	void WriteCircle(const Circle& circle);
	void WriteSector(const Sector& sector);

	//static const std::unordered_map<std::string, Waypoint::Type> IGCWaypointTable;
	static bool calculateArcs;
	//static CoordinateType coordinateType;
	//std::multimap<int, Waypoint>& waypoints;
	std::multimap<int,Waypoint*>& waypoints;
	bool varRotationClockwise;
	Geometry::LatLon varPoint;
	//double varWidth;
	std::ofstream file;
	int lastACline;
	bool lastPointWasDDMMSS;
	int lastLatD, lastLatM, lastLatS, lastLonD, lastLonM, lastLonS;
};
