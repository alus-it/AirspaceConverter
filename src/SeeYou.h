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
	static bool ParseLength(const std::string& text, int& len);
	static bool ParseFrequency(const std::string& text, float& freq);

	std::multimap<int,Waypoint*>& waypoints;
};
