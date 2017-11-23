//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2017 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <string>
#include <map>
#include <fstream>
#include "Geometry.h"

class Airspace;

class OpenAir {
friend class Point;
friend class Circle;
friend class Sector;

public:
	OpenAir(std::multimap<int, Airspace>& airspacesMap): airspaces(airspacesMap), varRotationClockwise(true), lastACline(-1) {}
	~OpenAir() {}
	bool Read(const std::string& fileName);
	bool Write(const std::string& fileName, const Geometry::Limits& limits);

private:
	static std::string& RemoveComments(std::string &s);
	static bool ParseDegrees(const std::string& dddmmss, double& deg);
	static bool ParseCoordinates(const std::string& text, Geometry::LatLon& point);
	static bool ParseAN(const std::string& line, Airspace& airspace);
	static bool ParseAltitude(const std::string& line, const bool isTop, Airspace& airspace);
	static bool ParseS (const std::string& line);
	static bool ParseT (const std::string& line);
	static bool ParseDP(const std::string& line, Airspace& airspace);
	bool ParseAC(const std::string& line, Airspace& airspace);
	bool ParseV(const std::string& line, Airspace& airspace);
	bool ParseDA(const std::string& line, Airspace& airspace);
	bool ParseDB(const std::string& line, Airspace& airspace);
	bool ParseDC(const std::string& line, Airspace& airspace);
	//bool ParseDY(const std::string& line, Airspace& airspace); // Airway not yet supported
	bool InsertAirspace(Airspace& airspace);
	void WriteHeader();
	bool WriteCategory(const Airspace& airsapce);
	void WriteLatLon(const Geometry::LatLon& point);
	void WritePoint(const Point& point);
	void WriteCircle(const Circle& circle);
	void WriteSector(const Sector& sector);

	std::multimap<int, Airspace>& airspaces;
	bool varRotationClockwise;
	Geometry::LatLon varPoint;
	//double varWidth;
	std::ofstream file;
	int lastACline;
};
