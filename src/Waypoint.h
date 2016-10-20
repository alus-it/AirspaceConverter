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

	Waypoint(const std::string& longName, const std::string& shortName, const std::string& countryCode, const double lat, const double lon, const int alt, const int style, const std::string& descr)
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

	inline static const bool IsTypeAirfield(const WaypointType& kind) { return kind >= airfieldGrass && kind <= airfieldSolid; }

	inline const std::string& GetName() const { return name; }
	inline const std::string& GetCode() const { return code; }
	inline const std::string& GetCountry() const { return country; }
	inline const double GetLatitude() const { return latitude; }
	inline const double GetLongitude() const { return longitude; }
	inline const int GetAltitude() const { return altitude; }
	inline const WaypointType GetType() const { return type; }
	inline const std::string& GetTypeName() const { return TypeName(type); }
	inline const std::string& GetDescription() const { return description; }
	inline const bool IsAirfield() const { return IsTypeAirfield(type); }

	inline static const std::string& TypeName(const WaypointType& type) { return TYPE_NAMES[type]; }

private:
	std::string name;
	std::string code;
	std::string country;
	double latitude; // [deg]
	double longitude; // [deg]
	int altitude; // [m]
	WaypointType type;
	std::string description;
	static const std::string TYPE_NAMES[];
};

