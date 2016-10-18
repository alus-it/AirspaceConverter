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

#include "CUPreader.h"
#include "AirspaceConverter.h"
#include "Waypoint.h"
#include "Airfield.h"
#include "Airspace.h"
#include "Geometry.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <cassert>

bool ParseLatitude(const std::string& text, double& lat) {
	if(text.length()!=9 || text.at(4)!='.') return false;
	lat = std::stoi(text.substr(0,2));
	lat += std::stod(text.substr(2,6))/60;
	const char sign = text.at(8);
	if (sign == 'S' || sign == 's') lat = -lat;
	else if (sign != 'N' && sign != 'n') return false;
	return true;
}

bool ParseLongitude(const std::string& text, double& lon) {
	if(text.length()!=10 || text.at(5)!='.') return false;
	lon = std::stoi(text.substr(0,3));
	lon += std::stod(text.substr(3,6))/60;
	const char sign = text.at(8);
	if (sign == 'W' || sign == 'w') lon = -lon;
	else if (sign != 'E' && sign != 'e') return false;
	return true;
}

bool ParseAltitude(const std::string& text, int& alt) {
	int pos = text.length()-1;
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
	double altitude = std::stod(text.substr(0,pos));
	if(feet) altitude *= Altitude::FEET2METER;
	alt = round(altitude);
	return true;
}

bool ParseLength(const std::string& text, int& len) {
	int pos = text.length()-1;
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
	double length = std::stod(text.substr(0,pos));
	if (length < 0) return false;
	if (nauticalMiles) length *= Geometry::NM2M;
	else if (statuteMiles) length *= Geometry::MI2M;
	len = round(length);
	return true;
}

bool CUPreader::ReadFile(const std::string& fileName, std::multimap<int,Waypoint>& output) {
	std::ifstream input(fileName);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open CUP input file: " + fileName, true);
		return false;
	}
	AirspaceConverter::LogMessage("Reading CUP file: " + fileName, false);

	int linecount = 0;

	while (!input.eof() && input.good())
	{
		// Get the line
		std::string sLine;
		std::getline(input, sLine);

		// Skip eventual header
		if(linecount++ == 0 && sLine == "name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc") continue;

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
		if(std::distance(tokens.begin(),tokens.end()) != 11) return false; // We expect only 11 fields

		// Long name
		boost::tokenizer<boost::escaped_list_separator<char> >::iterator token=tokens.begin();
		std::string name = *token;
		if (name.empty()) {
			AirspaceConverter::LogMessage(boost::str(boost::format("ERROR: while parsing CUP line %1d a name must be present: %2s") %linecount %sLine), true);
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
			AirspaceConverter::LogMessage(boost::str(boost::format("ERROR: while parsing CUP line %1d invalid latitude: %2s") %linecount %sLine), true);
			continue;
		}

		// Longitude
		token++;
		double longitude;
		if(!ParseLongitude(*token,longitude)) {
			AirspaceConverter::LogMessage(boost::str(boost::format("ERROR: while parsing CUP line %1d invalid longitude: %2s") %linecount %sLine), true);
			continue;
		}

		// Elevation
		token++;
		int altitude;
		if(!ParseAltitude(*token,altitude)) {
			AirspaceConverter::LogMessage(boost::str(boost::format("ERROR: while parsing CUP line %1d invalid elevation: %2s") %linecount %sLine), true);
			continue;
		}

		// Waypoint style
		token++;
		int type = std::stoi(*token);
		if(type <= Waypoint::undefined || type >= Waypoint::numOfWaypointTypes) {
			AirspaceConverter::LogMessage(boost::str(boost::format("ERROR: while parsing CUP line %1d invalid waypoint style: %2s") %linecount %sLine), true);
			continue;
		}

		// If it's an airfield...
		if(type <= Waypoint::airfieldGrass && type >= Waypoint::airfieldSolid) {
			// Runway direction
			token++;
			int runwayDir = std::stoi(*token);
			if(runwayDir < 0 || runwayDir > 360) {
				AirspaceConverter::LogMessage(boost::str(boost::format("ERROR: while parsing CUP line %1d invalid runway direction: %2s") %linecount %sLine), true);
				continue;
			}

			// Runway length
			token++;
			int runwayLength;
			if(!ParseLength(*token,runwayLength)) {
				AirspaceConverter::LogMessage(boost::str(boost::format("ERROR: while parsing CUP line %1d invalid runway length: %2s") %linecount %sLine), true);
				continue;
			}

			// Radio frequency
			token++;
			std::string radioFreq = *token;

			// Description
			token++;
			assert(token != tokens.end());
			std::string description = *token;

			// Build the airfield
			Airfield airfield(name, code, country, latitude, longitude, altitude, type, runwayDir, runwayLength, radioFreq, description);

			// Add it to the multimap
			output.insert(std::pair<int, Waypoint>(type, std::move(airfield)));
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
			Waypoint waypoint(name, code, country, latitude, longitude, altitude, type, description);

			// Add it to the multimap
			output.insert(std::pair<int, Waypoint>(type, std::move(waypoint)));
		}
	}
	return true;
}
