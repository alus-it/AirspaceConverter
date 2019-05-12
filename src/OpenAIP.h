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
	static bool ParseGeolocation(const boost::property_tree::ptree& parentNode, double &lat, double &lon, double &alt);
	static bool ParseContent(const boost::property_tree::ptree& parentNode, const std::string& tagName, std::string& outputString);
	static bool ParseAttribute(const boost::property_tree::ptree& node, const std::string& attributeName, std::string& outputString);
	static bool ParseValue(const boost::property_tree::ptree& parentNode, const std::string& tagName, double &value);
	static bool ParseMeasurement(const boost::property_tree::ptree& parentNode, const std::string&  tagName, char expectedUnit, double &value);

	bool ParseAirports(const boost::property_tree::ptree& airportsNode);
	bool ParseNavAids(const boost::property_tree::ptree& navAidsNode);
	bool ParseHotSpots(const boost::property_tree::ptree& hotSpotsNode);

	std::multimap<int,Airspace>& airspaces;
	std::multimap<int,Waypoint*>& waypoints;
};
