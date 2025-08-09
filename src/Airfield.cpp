//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "Airfield.hpp"
#include <cassert>

Airfield::Airfield(const std::string& longName, const std::string& shortName, const std::string& countryCode, const double lat, const double lon, const float alt, const int style, const int rwyDir, const int rwyLen, const int freq, const std::string& descr)
	: Waypoint(longName, shortName, countryCode, lat, lon, alt, style, descr)
	, runwayDir(rwyDir)
	, runwayLength(rwyLen)
	, radioFreq(freq) {
	assert(IsAirfield());
}

Airfield::Airfield(const std::string& longName, const std::string& shortName, const std::string& countryCode, const double lat, const double lon, const float alt, const int style, const std::string& descr)
	: Waypoint(longName, shortName, countryCode, lat, lon, alt, style, descr)
	, runwayDir(-1)
	, runwayLength(-1)
	, radioFreq(-1) {
	assert(IsAirfield());
}
