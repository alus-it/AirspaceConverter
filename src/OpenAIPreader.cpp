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

#include "OpenAIPreader.h"
#include "Airspace.h"
#include "AirspaceConverter.h"
#include <fstream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/tokenizer.hpp>

using boost::property_tree::ptree;

bool ReadAltitude(const ptree& node, Altitude& altitude) {
	try {
		ptree alt = node.get_child("ALT");
		std::string str = alt.get_child("<xmlattr>").get<std::string>("UNIT");
		bool isFeet(false), isFL(false);
		switch (str.length()) {
		case 1: // F
			if (str.at(0) == 'F') isFeet = true;
			break;
		case 2: //FL
			if (str.at(0) == 'F' && str.at(1) == 'L') isFL = true;
			break;
		default:
			break;
		}
		if (!isFeet && !isFL) return false;
		const int value = (int)(node.get<double>("ALT"));
		str = node.get_child("<xmlattr>").get<std::string>("REFERENCE");
		if (str.length() == 3) {
			switch (str.at(0)) {
			case 'M': // MSL Main sea level
				if (str.at(1) == 'S' && str.at(2) == 'L') {
					if (!isFeet) return false;
					altitude.SetAltFtMSL(value);
					return true;
				}
				break;
			case 'S': //STD Standard atmosphere
				if (str.at(1) == 'T' && str.at(2) == 'D') {
					if (!isFL) return false;
					altitude.SetFlightLevel(value);
					return true;
				}
				break;
			case 'G': // GND Ground
				if (str.at(1) == 'N' && str.at(2) == 'D') {
					if (!isFeet) return false;
					altitude.SetAltFtGND(value);
					return true;
				}
				break;
			default:
				break;
			}
		}
		
	} catch (...) {}
	return false;
}

bool OpenAIPreader::ReadFile(const std::string& fileName, std::multimap<int, Airspace>& output) {	
	std::ifstream input(fileName);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open OpenAIP input file: " + fileName, true);
		return false;
	}
	AirspaceConverter::LogMessage("Reading OpenAIP file: " + fileName, false);
	ptree tree;
	read_xml(input, tree);
	input.close();
	ptree root;
	try {
		root = tree.get_child("OPENAIP");
		double value = root.get_child("<xmlattr>").get<double>("DATAFORMAT");
		if (value != 1.1) {
			AirspaceConverter::LogMessage("ERROR: DATAFORMAT attribute missing or not at the expected version 1.1", true);
			return false;
		}

		// for all children of AIRSPACES tag
		for (ptree::value_type const& asp : root.get_child("AIRSPACES")) {
			if (asp.first != "ASP") continue;
			
			// Airspace category
			std::string str = asp.second.get_child("<xmlattr>").get<std::string>("CATEGORY");
			Airspace::Type type = Airspace::UNDEFINED;
			int len = (int)str.length();
			if (len>0) switch (str.at(0)) {
				case 'A':
					if (len == 1) type = Airspace::CLASSA; // A class airspace
					break;
				case 'B':
					if (len == 1) type = Airspace::CLASSB; // B class airspace
					break;
				case 'C':
					if (len == 1) type = Airspace::CLASSC; // C class airspace
					else if (str == "CTR") type = Airspace::CTR; // CTR airspace
					break;
				case 'D':
					if (len == 1) type = Airspace::CLASSD; // D class airspace
					else if (str == "DANGER") type = Airspace::DANGER; // Dangerous area
					break;
				case 'E':
					if (len == 1) type = Airspace::CLASSE; // E class airspace
					break;
				case 'F':
					if (len == 1) type = Airspace::CLASSF; // F class airspace
					else if (str == "FIR") type = Airspace::FIR; //FIR
					break;
				case 'G':
					if (len == 1) type = Airspace::CLASSG; // G class airspace
					else if (str == "GLIDING") type = Airspace::GLIDING;
					break;
				case 'O':
					if (str == "OTH") type = Airspace::OTH;
					break;
				case 'P':
					if (str == "PROHIBITED") type = Airspace::PROHIBITED; // Prohibited area
					break;
				case 'R':
					if (str == "RESTRICTED") type = Airspace::RESTRICTED; // Restricted area
					else if (str == "RMZ") type = Airspace::RMZ; //RMZ
					break;
				case 'T':
					if (len == 3 && str.at(1) == 'M') {
						if (str.at(2) == 'A') type = Airspace::TMA;
						else if (str.at(2) == 'Z') type = Airspace::TMZ;
					}
					break;
				case 'W':
					if (str == "WAVE") type = Airspace::WAVE; //WAVE
					break;
				case 'U':
					if (str == "UIR") type = Airspace::UIR; //UIR
					break;
				default:
					break;
				} else continue;
				if (type == Airspace::UNDEFINED) {
					AirspaceConverter::LogMessage("Warning: skipping ASP with unknown/undefined CATEGORY attribute: " + str, false);
					continue;
				}
				Airspace airspace(type);

				// Airspace name
				str = asp.second.get<std::string>("NAME");
				airspace.SetName(str);

				// Airspace top altitude
				ptree node = asp.second.get_child("ALTLIMIT_TOP");
				Altitude alt;
				if (ReadAltitude(node, alt)) airspace.SetTopAltitude(alt);
				else return false;

				// Airspace bottom altitude
				node = asp.second.get_child("ALTLIMIT_BOTTOM");
				if (ReadAltitude(node, alt)) airspace.SetBaseAltitude(alt);
				else return false;

				//Geometry
				node = asp.second.get_child("GEOMETRY");

				// Polygon (the only one supported for now)
				str = node.get<std::string>("POLYGON");
				double lat = Geometry::LatLon::UNDEF_LAT, lon = Geometry::LatLon::UNDEF_LON;
				boost::char_separator<char> sep(", ");
				boost::tokenizer<boost::char_separator<char> > tokens(str, sep);
				bool expectedLon(true), error(false);
				try {
					for (const std::string& c : tokens) {
						if (expectedLon) { // Beware that here the longitude comes first!
							lon = std::stod(c);
							if (!Geometry::LatLon::IsValidLon(lon)) {
								error = true;
								break;
							}
							expectedLon = false;
						} else {
							lat = std::stod(c);
							if (!Geometry::LatLon::IsValidLat(lat)) {
								error = true;
								break;
							}
							expectedLon = true;
							airspace.AddSinglePointOnly(lat, lon);
						}
					}
				} catch (...) {
					error = true;
				}
				if (error || !expectedLon) {
					AirspaceConverter::LogMessage("Warning: skipping invalid coordinates.", false);
					continue;
				}

				// Ensure that the polygon is closed (it should be already but can happen).....
				airspace.ClosePoints();

				// The number of points must be at least 3+1 (plus the closing one)
				assert(airspace.GetNumberOfPoints() > 3);
				if (airspace.GetNumberOfPoints() <= 3) {
					AirspaceConverter::LogMessage("Warning: skipping airspace with less tha 3 points.", false);
					continue;
				}

				// Add the new airspace
				output.insert(std::pair<int, Airspace>(airspace.GetType(), std::move(airspace)));

			} // for each ASP
			return true;
	} catch (...) {
		AirspaceConverter::LogMessage("ERROR: Exception while parsing OpenAIP file.", true);
		assert(false);
	}
	return false;
}
