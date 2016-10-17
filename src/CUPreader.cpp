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
#include "Airfield.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <cassert>

bool ParseLatitude(const std::string& text, double& lat) {
	//TODO:...
	return false;
}

bool ParseLongitude(const std::string& text, double& lon) {
	//TODO:...
	return false;
}

bool ParseAltitude(const std::string& text, int& alt) {
	//TODO:...
	return false;
}

bool ParseLength(const std::string& text, int& len) {
	//TODO:...
	return false;
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
		std::string sLine;
		std::getline(input, sLine);
		++linecount;

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
		int numOfFields = std::distance(tokens.begin(),tokens.end());
		if(numOfFields != 11) return false; // We expect only 11 fields

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

			// Build the airfield
			Waypoint waypoint(name, code, country, latitude, longitude, altitude, type, description);

			// Add it to the multimap
			output.insert(std::pair<int, Waypoint>(type, std::move(waypoint)));
		}
	}
	return true;
}
