//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2024 Alberto Realis-Luc
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
	static bool ParseStyle(const std::string& text, int& type);
	static bool ParseRunwayDir(const std::string& text, int& dir);
	static bool ParseRunwayLength(const std::string& text, int& length);
	static bool ParseAirfieldFrequencies(const std::string& text, int& mainFreqHz, int& secondaryFreqHz);
	static bool ParseOtherFrequency(const std::string& text, const int type, int& freqHz);

	static const std::string defaultHeader;
	std::multimap<int,Waypoint*>& waypoints;
};
