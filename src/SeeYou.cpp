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

#include "AirspaceConverter.h"
#include "Waypoint.h"
#include "Airfield.h"
#include "Airspace.h"
#include "Geometry.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <cmath>
#include <cassert>
#include "SeeYou.h"

bool ParseLatitude(const std::string& text, double& lat) {
	const int len = (int)text.length();
	if(len < 5) return false;
	try {
		lat = std::stoi(text.substr(0,2));
		lat += std::stod(text.substr(2,len-3))/60;
	} catch(...) {
		return false;
	}
	const char sign = text.back();
	if (sign == 'S' || sign == 's') lat = -lat;
	else if (sign != 'N' && sign != 'n') return false;
	return true;
}

bool ParseLongitude(const std::string& text, double& lon) {
	const int len = (int)text.length();
	if(len < 6) return false;
	try {
		lon = std::stoi(text.substr(0,3));
		lon += std::stod(text.substr(3,len-4))/60;
	} catch (...) {
		return false;
	}
	const char sign = text.back();
	if (sign == 'W' || sign == 'w') lon = -lon;
	else if (sign != 'E' && sign != 'e') return false;
	return true;
}

bool ParseAltitude(const std::string& text, int& alt) {
	int pos = (int)text.length() - 1;
	if(pos == 0 && text.front()=='0') {
		alt = 0;
		return true;
	}
	if(pos<1) return false;
	bool feet = false;
	if(text.back() == 't' || text.back() == 'T') pos--;
	switch(text.at(pos)) {
		case 'm':
		case 'M':
			break;
		case 'f':
		case 'F':
			feet = true;
			break;
		default:
			return false;
	}
	try {
		double altitude = std::stod(text.substr(0,pos));
		if(feet) altitude *= Altitude::FEET2METER;
		alt = (int)std::round(altitude);
	} catch(...) {
		return false;
	}
	return true;
}

bool ParseLength(const std::string& text, int& len) {
	int pos = (int)text.length() - 1;
	if(pos<2) return false;
	bool nauticalMiles = false, statuteMiles = false;
	if(text.back() == 'm' || text.back() == 'M') {
		if(text.at(pos) == 'n' || text.at(pos) == 'N') {
			pos--;
			nauticalMiles = true;
		}
	} else if((text.at(pos-1) == 'm' || text.at(pos-1) == 'M') && (text.back() == 'l' || text.back() == 'L' || text.back() == 'i' || text.back() == 'I')) {
		pos--;
		statuteMiles = true;
	} else return false;
	try {
		double length = std::stod(text.substr(0,pos));
		if (length < 0) return false;
		if (nauticalMiles) length *= Geometry::NM2M;
		else if (statuteMiles) length *= Geometry::MI2M;
		len = (int)std::round(length);
	} catch (...) {
		return false;
	}
	return true;
}

bool SeeYou::ReadFile(const std::string& fileName, std::multimap<int,Waypoint*>& output) {
	std::ifstream input(fileName, std::ios::binary);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open CUP input file: " + fileName, true);
		return false;
	}
	AirspaceConverter::LogMessage("Reading CUP file: " + fileName, false);
	int linecount = 0;
	std::string sLine;
	bool isCRLF = false, CRLFwarningGiven = false, firstWaypointFound = false;

	while (!input.eof() && input.good()) {

		// Get the line
		AirspaceConverter::SafeGetline(input, sLine, isCRLF);
		linecount++;

		// Verify line ending
		if (!CRLFwarningGiven && !isCRLF) {
			AirspaceConverter::LogMessage(boost::str(boost::format("WARNING on line %1d: not valid Windows style end of line (expected CR LF).") % linecount), true);
			AirspaceConverter::LogMessage("This warning will be not repeated for further lines not terminated with CR LF of this CUP file.", false);

			// CUP files may contain thousends of WPs we don't want to print this warning all the time
			CRLFwarningGiven = true;
		}

		// Skip eventual header
		if (!firstWaypointFound && sLine.find("name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc") == 0) continue;

		// Directly skip empty lines
		if (sLine.empty()) continue;

		// Remove front spaces
		boost::algorithm::trim_left(sLine);

		// If it was a line with only spaces skip it
		if (sLine.empty()) continue;

		// Then directly skip full comment line
		if (sLine.front() == '*') continue;

		// Remove back spaces
		boost::algorithm::trim_right(sLine);

		// Skip too short lines
		if (sLine.size() <= 10) continue; // At least ten commas should be there

		// Check if we arrived to the task section, if yes we're done
		if (sLine == "-----Related Tasks-----") break;

		// Tokenize with quotes
		boost::tokenizer<boost::escaped_list_separator<char> > tokens(sLine); // default separator:',', default quote:'"', default escape char:'\'
		if(std::distance(tokens.begin(),tokens.end()) != 11) { // We expect only 11 fields
			if (firstWaypointFound) AirspaceConverter::LogMessage(boost::str(boost::format("ERROR on line %1d: expected 11 fields: %2s") %linecount %sLine), true);
			continue;
		}

		// Long name
		boost::tokenizer<boost::escaped_list_separator<char> >::iterator token=tokens.begin();
		std::string name = *token;
		if (name.empty()) {
			if (firstWaypointFound) AirspaceConverter::LogMessage(boost::str(boost::format("ERROR on line %1d: a name must be present: %2s") %linecount %sLine), true);
			continue;
		}

		// Code
		token++;
		std::string code = *token;

		// Country code
		token++;
		std::string country = *token;

		// Latitude
		token++;
		double latitude;
		if(!ParseLatitude(*token,latitude)) {
			if (firstWaypointFound) AirspaceConverter::LogMessage(boost::str(boost::format("ERROR on line %1d: invalid latitude: %2s") %linecount %(*token)), true);
			continue;
		}

		// Longitude
		token++;
		double longitude;
		if(!ParseLongitude(*token,longitude)) {
			if (firstWaypointFound) AirspaceConverter::LogMessage(boost::str(boost::format("ERROR on line %1d: invalid longitude: %2s") %linecount %(*token)), true);
			continue;
		}

		// Elevation
		token++;
		int altitude = 0;
		if(!ParseAltitude(*token,altitude))
			if (firstWaypointFound) AirspaceConverter::LogMessage(boost::str(boost::format("WARNING on line %1d: invalid elevation: %2s, assuming AMSL") %linecount %(*token)), true);

		// Waypoint style
		token++;
		int type = Waypoint::undefined;
		try {
			type = std::stoi(*token);
		} catch(...) {}
		if(type <= Waypoint::undefined || type >= Waypoint::numOfWaypointTypes) {
			if (firstWaypointFound) AirspaceConverter::LogMessage(boost::str(boost::format("WARNING on line %1d: invalid waypoint style: %2s, assuming normal") %linecount %(*token)), true);
			type = Waypoint::normal;
		}

		// If it's an airfield...
		if(Waypoint::IsTypeAirfield((Waypoint::WaypointType)type)) {
			// Runway direction
			token++;
			int runwayDir = -1;
			try {
				runwayDir = std::stoi(*token);
			} catch(...) {}
			if(runwayDir < 0 || runwayDir > 360) {
				runwayDir = -1; // make sure it is set at -1
				if (firstWaypointFound) AirspaceConverter::LogMessage(boost::str(boost::format("WARNING on line %1d: invalid runway direction: %2s") % linecount % (*token)), true);
			}

			// Runway length
			token++;
			int runwayLength = -1;
			if(!ParseLength(*token,runwayLength))
				if (firstWaypointFound) AirspaceConverter::LogMessage(boost::str(boost::format("WARNING on line %1d: invalid runway length: %2s") %linecount %(*token)), true);

			// Radio frequency
			token++;
			std::string radioFreq = *token;

			// Description
			token++;
			assert(token != tokens.end());
			std::string description = *token;

			// Build the airfield
			Airfield* airfield = new Airfield(name, code, country, latitude, longitude, altitude, type, runwayDir, runwayLength, radioFreq, description);

			// Add it to the multimap
			output.insert(std::pair<int, Waypoint*>(type, (Waypoint*)airfield));
		} else {
			// Skip the airfield's fields
			token++;
			token++;
			token++;

			// Description
			token++;
			assert(token != tokens.end());
			std::string description = *token;

			// Build the waypoint
			Waypoint* waypoint = new Waypoint(name, code, country, latitude, longitude, altitude, type, description);

			// Add it to the multimap
			output.insert(std::pair<int, Waypoint*>(type, waypoint));
		}

		// Make sure that at this point we already found a valid waypoint so the header is not anymore expected
		if (!firstWaypointFound) firstWaypointFound = true;
	}
	return true;
}
