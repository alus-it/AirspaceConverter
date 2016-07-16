//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "PFMwriter.h"
#include "AirspaceConverter.h"
#include "Airspace.h"
#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <cassert>

const std::string PFMwriter::types[] = {
	{ "0x0a" }, //CLASSA
	{ "0x0a" }, //CLASSB
	{ "0x0a" }, //CLASSC
	{ "0x0a" }, //CLASSD
	{ "0x0a" }, //CLASSE
	{ "0x0a" }, //CLASSF
	{ "0x0a" }, //CLASSG
	{ "0x0a" }, //DANGER
	{ "0x0a" }, //PROHIBITED
	{ "0x0a" }, //RESTRICTED
	{ "0x0a" }, //CTR
	{ "0x0a" }, //TMA
	{ "0x0a" }, //TMZ
	{ "0x0a" }, //RMZ
	{ "0x0a" }, //FIR
	{ "0x0a" }, //UIR
	{ "0x0a" }, //OTH
	{ "0x0a" }, //GLIDING
	{ "0x0a" }, //NOGLIDER
	{ "0x0a" }, //WAVE
	{ "0x0a" }  //UNKNOWN
};

const std::string PFMwriter::MakeLabel(const Airspace& airspace) {
	std::stringstream ss;
	ss << airspace.GetCategoryName() << " "
		<< airspace.GetTopAltitude().ToString() << " - "
		<< airspace.GetBaseAltitude().ToString() << " "
		<< airspace.GetName();
	std::string label(ss.str());
	if(label.length()>80) label.resize(80);
	return label;
}

void PFMwriter::WriteHeader(const std::string& filename) {
	for(const std::string& line: AirspaceConverter::disclaimer) file << ";" << line << "\n";
	file << "\n[IMG ID]\n" //section identifier
		<< "ID=62831853\n" // unique identifier: 2 PI
		<< "Name=" << filename << "/n" // map name
		<< "LBLcoding=6\n"
		<< "Codepage=1252\n"
		<< "Datum=W84\n"
		<< "Transparent=Y\n"
		<< "MG=N\n"
		<< "Numbering=N\n"
		<< "Routing=N\n"
		<< "Lock=N\n"
		<< "ProductCode=1\n"
		<< "CopyRight=AirspaceConverter www.alus.it/AirspaceConverter\n"
		<< "Elevation=m\n"
		<< "POIIndex=N\n"
		<< "TreSize=1311\n"
		<< "RgnLimit=1024\n"
		<< "PreProcess=F\n"
		<< "Levels=5\n"
		<< "Level0=24\n"
		<< "Level1=21\n"
		<< "Level2=18\n"
		<< "Level3=15\n"
		<< "Level4=12\n"
		<< "Zoom0=0\n"
		<< "Zoom1=1\n"
		<< "Zoom2=2\n"
		<< "Zoom3=3\n"
		<< "Zoom4=4\n"
		<< "[END]\n\n";
}

bool PFMwriter::WriteFile(const std::string& filename, const std::multimap<int, Airspace>& airspaces)
{
	// Check if has the right extension
	if (!boost::iequals(boost::filesystem::path(filename).extension().string(), ".mp")) {
		AirspaceConverter::LogMessage("ERROR: Expected MP extension but found: " + boost::filesystem::path(filename).extension().string(), true);
		return false;
	}

	if (file.is_open()) file.close();
	file.open(filename);
	if (!file.is_open() || file.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open output file: " + filename, true);
		return false;
	}
	AirspaceConverter::LogMessage("Writing output file: " + filename, false);

	WriteHeader(filename);

	// For each category
	for (int t = Airspace::CLASSA; t <= Airspace::UNKNOWN; t++) {
		// First verify if there are airspaces of that class
		if (airspaces.count(t) == 0) continue;
		const auto filtered = airspaces.equal_range(t);
		for (auto it = filtered.first; it != filtered.second; ++it) {
			const Airspace& a = it->second;

			assert(a.GetNumberOfPoints() > 3);
			assert(a.GetFirstPoint()==a.GetLastPoint());

			file << "[POLYGON]\n"
				<< "Type="<< types[a.GetType()] <<"\n"
				<< "Label="<<MakeLabel(a)<<"\n"
				<< "Data0=";

			double lat,lon;
			for (unsigned int i=0; i<a.GetNumberOfPoints()-1; i++) {
				a.GetPointAt(i).GetLatLon(lat,lon);
				file << "(" << lat << "," << lon << "),";
			}
			a.GetLastPoint().GetLatLon(lat,lon);
			file << "(" << lat << "," << lon << ")\n";
			file<< "EndLevel=4\n"
				"[END]\n\n";
		}
	}
	file.close();
	return true;
}
