//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <string>
#include <map>

class Waypoint;

class SeeYou {

public:
	SeeYou(std::multimap<int,Waypoint*>& waypointsMap);
	~SeeYou() {}
	bool Read(const std::string& fileName);
	bool Write(const std::string& fileName);

private:
	static bool ParseLatitude(const std::string& text, double& lat);
	static bool ParseLongitude(const std::string& text, double& lon);
	static bool ParseAltitude(const std::string& text, float& alt);
	static int ParseRunwayDir(const std::string& text);
	static int ParseRunwayLength(const std::string& text);
	static float ParseAirfieldFrequencies(const std::string& text, float& secondaryFreq);
	static float ParseOtherFrequency(const std::string& text, const int type);

	std::multimap<int,Waypoint*>& waypoints;
};
