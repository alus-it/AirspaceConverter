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

#include "Waypoint.h"
#include "Geometry.h"
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

Waypoint::Waypoint(const std::string& longName, const std::string& shortName, const std::string& countryCode, const double lat, const double lon, const int alt, const int style, const std::string& descr)
		: name(longName)
		, code(shortName)
		, country(countryCode)
		, latitude(lat)
		, longitude(lon)
		, altitude(alt)
		, type((WaypointType)style)
		, description(descr) {
		assert(Geometry::LatLon::IsValidLat(latitude) && Geometry::LatLon::IsValidLat(longitude));
	}

