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

#include "Airfield.h"
#include <cassert>

Airfield::Airfield(const std::string& longName, const std::string& shortName, const std::string& countryCode, const double lat, const double lon, const int alt, const int style, const int rwyDir, const int rwyLen, const std::string& freq, const std::string& descr)
	: Waypoint(longName, shortName, countryCode, lat, lon, alt, style, descr)
	, runwayDir(rwyDir)
	, runwayLength(rwyLen)
	, radioFreq(freq) {
	assert(IsAirfield());
}

