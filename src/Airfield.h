//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Copyright   : (C) 2016-2017 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include "Waypoint.h"

class Airfield: public Waypoint {
public:
	Airfield(const std::string& longName, const std::string& shortName, const std::string& countryCode, const double lat, const double lon, const int alt, const int style, const int rwyDir, const int rwyLen, const std::string& freq, const std::string& descr);
	~Airfield() {}

	inline int GetRunwayDir() const { return runwayDir; }
	inline int GetRunwayLength() const { return runwayLength; }
	inline const std::string& GetRadioFrequency() const { return radioFreq; }

private:
	int runwayDir;    // [deg]
	int runwayLength; // [m]
	std::string radioFreq;

};

