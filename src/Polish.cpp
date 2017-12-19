//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2018 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "Polish.h"
#include "AirspaceConverter.h"
#include "Airspace.h"
#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <cassert>

/* TODO: customized types
const int PFMwriter::types[] = {
	0x60, //CLASSA
	0x61, //CLASSB
	0x69, //CLASSC
	0x62, //CLASSD
	0x66, //CLASSE
	0x63, //CLASSF
	0x69, //CLASSG
	0x63, //DANGER
	0x66, //PROHIBITED
	0x61, //RESTRICTED
	0x61, //CTR
	0x64, //TMA
	0x60, //TMZ
	0x60, //RMZ
	0x01, //FIR
	0x01, //UIR
	0x01, //OTH
	0x69, //GLIDING
	0x62, //NOGLIDER
	0x69, //WAVE
	0x01,  //UNKNOWN
	0x01  //UNDEFINED
};
*/

const std::string Polish::MakeLabel(const Airspace& airspace) {
	std::stringstream ss;
	ss << airspace.GetCategoryName() << " "
		<< airspace.GetTopAltitude().ToString() << " - "
		<< airspace.GetBaseAltitude().ToString() << " "
		<< airspace.GetName();
	std::string label(ss.str());
	if(label.length()>80) label.resize(80);
	return label;
}

void Polish::WriteHeader(const std::string& filename) {
	for(const std::string& line: AirspaceConverter::disclaimer) file << ";" << line << "\n";
	file << "\n[IMG ID]\n" //section identifier
		<< "ID=62831853\n" // unique identifier: 2 PI
		<< "Name=" << boost::filesystem::path(filename).stem().string() << "\n" // map name
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

bool Polish::Write(const std::string& filename, const std::multimap<int, Airspace>& airspaces) {
	if (airspaces.empty()) {
		AirspaceConverter::LogMessage("Polish output: no airspace, nothing to write", false);
		return false;
	}

	// Check if has the right extension
	if (!boost::iequals(boost::filesystem::path(filename).extension().string(), ".mp")) {
		AirspaceConverter::LogMessage("ERROR: Expected MP extension but found: " + boost::filesystem::path(filename).extension().string(), true);
		return false;
	}

	if (file.is_open()) file.close();
	file.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file.is_open() || file.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open output file: " + filename, true);
		return false;
	}
	AirspaceConverter::LogMessage("Writing output file: " + filename, false);

	WriteHeader(filename);

	// Go trough all airspaces
	for (const std::pair<const int,Airspace>& pair : airspaces)
	{
		// Get the airspace
		const Airspace& a = pair.second;

		// Just a couple if assertions
		assert(a.GetNumberOfPoints() > 3);
		assert(a.GetFirstPoint()==a.GetLastPoint());

		// Determine if it's a POLYGON or a POLYLINE
		if (a.GetType() == Airspace::PROHIBITED || a.GetType() == Airspace::CTR || a.GetType() == Airspace::DANGER) {
			file << "[POLYGON]\n"
				//<< "Type="<< types[a.GetType()] <<"\n"; //TODO...
				<< "Type=0x18" <<"\n";
		} else {
			file << "[POLYLINE]\n"
				<< "Type=0x07\n"; //TODO....
		}

		// Add the label
		file << "Label="<<MakeLabel(a)<<"\n";

		file << "Levels=3\n";

		// Insert all the points
		file << "Data0=";
		double lat,lon;
		for (unsigned int i=0; i<a.GetNumberOfPoints()-1; i++) {
			a.GetPointAt(i).GetLatLon(lat,lon);
			file << "(" << lat << "," << lon << "),";
		}
		a.GetLastPoint().GetLatLon(lat,lon);
		file << "(" << lat << "," << lon << ")\n";

		//file<< "EndLevel=4\n";

		// Close the element
		file << "[END]\n\n";
	}
	file.close();
	return true;
}
