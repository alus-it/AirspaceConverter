//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2022 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "Waypoint.h"
#include <cassert>

const std::string Waypoint::TYPE_NAMES[] = {
	"UNDEFINED",
	"Normal",
	"Airfield grass",
	"Outlanding",
	"Glider site",
	"Airfield solid",
	"Mount pass",
	"Mount top",
	"Antenna",
	"VOR",
	"NDB",
	"Cool tower",
	"Dam",
	"Tunnel",
	"Bridge",
	"Power plant",
	"Castle",
	"Intersection"
};

Waypoint::Waypoint(const std::string& longName, const std::string& shortName, const std::string& countryCode, const double lat, const double lon, const float alt, const int style, const std::string& descr)
	: pos(lat,lon)
	, name(longName)
	, code(shortName)
	, country(countryCode)
	, altitude(alt)
	, type((WaypointType)style)
	, otherFreq(0)
	, description(descr) {
	assert(pos.IsValid());
}
