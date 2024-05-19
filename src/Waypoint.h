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
#include "Geometry.h"

class Waypoint {

public:
	enum WaypointType {
		unknown = 0,
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

	Waypoint(const std::string& longName, const std::string& shortName, const std::string& countryCode, const double lat, const double lon, const float alt, const int style, const std::string& descr);

	virtual ~Waypoint() {}

	inline static bool IsTypeAirfield(const Waypoint::WaypointType& kind) { return kind >= airfieldGrass && kind <= airfieldSolid; }
	inline static bool IsTypeAirfield(const int& kind) { return kind >= airfieldGrass && kind <= airfieldSolid; }

	inline const std::string& GetName() const { return name; }
	inline const std::string& GetCode() const { return code; }
	inline const std::string& GetCountry() const { return country; }
	inline const Geometry::LatLon& GetPosition() const { return pos; }
	inline double GetLatitude() const { return pos.Lat(); }
	inline double GetLongitude() const { return pos.Lon(); }
	inline float GetAltitude() const { return altitude; }
	inline WaypointType GetType() const { return type; }
	inline const std::string& GetTypeName() const { return TypeName(type); }
	inline const std::string& GetDescription() const { return description; }
	inline bool IsAirfield() const { return IsTypeAirfield(type); }
	inline void SetOtherFrequency(const int freq) { otherFreq = freq; }
	inline bool HasOtherFrequency() const { return otherFreq > 0; }
	inline int GetOtherFrequency() const { return otherFreq; }

	inline static const std::string& TypeName(const WaypointType& type) { return TYPE_NAMES[type]; }
	inline static const std::string& TypeName(const int& type) { return TYPE_NAMES[type]; }

private:
	Geometry::LatLon pos;
	std::string name;
	std::string code;
	std::string country;
	float altitude; // [m]
	WaypointType type;
	int otherFreq; // [Hz] frequency for VOR NDB or secondary radio frequency for airports
	std::string description;
	static const std::string TYPE_NAMES[];
};

