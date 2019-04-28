//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <string>
#include <map>
#include <boost/property_tree/ptree_fwd.hpp>

class Airspace;
class Waypoint;
class Altitude;

class OpenAIP {

public:
	OpenAIP(std::multimap<int, Airspace>& airspacesMap, std::multimap<int,Waypoint*>& waypointsMap);
	~OpenAIP() {}
	bool ReadAirspaces(const std::string& fileName);
	bool ReadWaypoints(const std::string& fileName);

private:
	static bool ParseAltitude(const boost::property_tree::ptree& node, Altitude& altitude);

	std::multimap<int,Airspace>& airspaces;
	std::multimap<int,Waypoint*>& waypoints;
};
