//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <string>

class Waypoint {

public:
	enum WaypointType {
		undefined = 0,
		normal = 1,
		airfieldGrass,
		outlanding,
		gliderSite,
		airfieldSolid,
		mtPass,
		mtTop,
		sender,
		VOR,
		NDB,
		coolTower,
		dam,
		tunnel,
		bridge,
		powerPlant,
		castle,
		intersection,
		numOfWaypointTypes
	};

	Waypoint(std::string& longName, std::string& shortName, std::string& countryCode, double lat, double lon, double alt, int style, std::string& descr)
		: name(longName)
		, code(shortName)
		, country(countryCode)
		, latitude(lat)
		, longitude(lon)
		, altitude(alt)
		, type((WaypointType)style)
		, description(descr) {
	}

	virtual ~Waypoint() {}

private:
	std::string name;
	std::string code;
	std::string country;
	double latitude; // [deg]
	double longitude; // [deg]
	int altitude; // [m]
	WaypointType type;
	std::string description;
};

