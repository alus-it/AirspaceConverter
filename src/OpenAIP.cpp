//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2020 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "OpenAIP.h"
#include "Airspace.h"
#include "AirspaceConverter.h"
#include "Waypoint.h"
#include "Airfield.h"
#include <cmath>
#include <fstream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>

using boost::property_tree::ptree;

OpenAIP::OpenAIP(std::multimap<int, Airspace>& airspacesMap, std::multimap<int,Waypoint*>& waypointsMap):
	airspaces(airspacesMap),
	waypoints(waypointsMap) {
}

bool OpenAIP::ParseAltitude(const ptree& node, Altitude& altitude) {
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
					altitude.SetAltFt(value);
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
					altitude.SetAltFt(value, false);
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

bool OpenAIP::ReadAirspaces(const std::string& fileName) {
	std::ifstream input(fileName);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogError("Unable to open openAIP airspace input file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Reading openAIP airspace file: " + fileName);
	ptree tree;
	read_xml(input, tree);
	input.close();
	ptree root;
	try {
		root = tree.get_child("OPENAIP");
		double value = root.get_child("<xmlattr>").get<double>("DATAFORMAT");
		if (value != 1.1) {
			AirspaceConverter::LogError("DATAFORMAT attribute missing or not at the expected version 1.1");
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
					AirspaceConverter::LogWarning("skipping ASP with unknown/undefined CATEGORY attribute: " + str);
					continue;
				}
				Airspace airspace(type);

				// Airspace name
				str = asp.second.get<std::string>("NAME");
				airspace.SetName(str);

				// Airspace top altitude
				ptree node = asp.second.get_child("ALTLIMIT_TOP");
				Altitude alt;
				if (ParseAltitude(node, alt)) airspace.SetTopAltitude(alt);
				else {
					AirspaceConverter::LogWarning("skipping airspace with invalid or missing ALTLIMIT_TOP attribute: " + airspace.GetName());
					continue;
				}

				// Airspace bottom altitude
				node = asp.second.get_child("ALTLIMIT_BOTTOM");
				if (ParseAltitude(node, alt)) airspace.SetBaseAltitude(alt);
				else {
					AirspaceConverter::LogWarning("skipping airspace with invalid or missing ALTLIMIT_BOTTOM attribute: " + airspace.GetName());
					continue;
				}

				// Extra check on consistency of altitude levels
				if (airspace.GetTopAltitude() <= airspace.GetBaseAltitude())
					AirspaceConverter::LogWarning("detected airspace with top and base equal or inverted: " + airspace.GetName());

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
					AirspaceConverter::LogWarning("skipping airspace with invalid coordinates: " + airspace.GetName());
					continue;
				}

				// Ensure that the polygon is closed (it should be already, but can still happen).....
				if (!airspace.ClosePoints()) {
					AirspaceConverter::LogWarning("skipping airspace with less than 3 points: : " + airspace.GetName());
					continue;
				}

				// The number of points must be at least 3+1 (plus the closing one)
				assert(airspace.GetNumberOfPoints() > 3);

				// Verify that the current airspace it not already existing in our collection (apparently this happens in in the same openAIP file)
				bool found(false);

				// Filter only on airspaces of the same type
				const auto filtered = airspaces.equal_range(airspace.GetType());
				for (auto it = filtered.first; it != filtered.second && !found; ++it) {
					if (it->second == airspace) {
						found = true;
						AirspaceConverter::LogWarning("Skipping existing airspace: " + airspace.GetName() + " already known as: " + it->second.GetName());
					}
				}

				// If it is not already present in our collection add the new airspace
				if (!found) airspaces.insert(std::pair<int, Airspace>(airspace.GetType(), std::move(airspace)));

			} // for each ASP
			return true;
	} catch (...) {
		AirspaceConverter::LogError("Exception while parsing openAIP file.");
		assert(false);
	}
	return false;
}

bool OpenAIP::ReadWaypoints(const std::string& fileName) {
	std::ifstream input(fileName);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogError("Unable to open openAIP waypoint input file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Reading openAIP waypoint file: " + fileName);
	ptree tree;
	read_xml(input, tree);
	input.close();
	ptree root;
	bool wptFound(false);
	try {
		root = tree.get_child("OPENAIP");
		double value = root.get_child("<xmlattr>").get<double>("DATAFORMAT");
		if (value != 1.1) {
			AirspaceConverter::LogError("DATAFORMAT attribute missing or not at the expected version 1.1");
			return false;
		}

		// Look for the 'root' of airports: WAYPOINTS tag
		if(root.count("WAYPOINTS") > 0) wptFound = ParseAirports(root.get_child("WAYPOINTS"));

		// Look for the 'root' of navigation aids: NAVAIDS tag
		if(root.count("NAVAIDS") > 0) wptFound = wptFound || ParseNavAids(root.get_child("NAVAIDS"));

		// Look for the 'root' of hot spots: HOTSPOTS tag
		if(root.count("HOTSPOTS") > 0) AirspaceConverter::LogWarning("openAIP hotspot file not parsed because not supported yet."); //TODO: wptFound = wptFound || ParseNavAids(root.get_child("NAVAIDS"));
	} catch (...) {
		AirspaceConverter::LogError("Exception while parsing openAIP waypoint file: " + fileName);
		assert(false);
		return false;
	}
	if(!wptFound) AirspaceConverter::LogWarning("Waypoints of any kind not found in this OpenAIP file: " + fileName);
	return wptFound;
}

bool OpenAIP::ParseAirports(const ptree& airportsNode) {
	int numOfAirports = airportsNode.count("AIRPORT"); //count number of airports in the file
	if (numOfAirports < 1) {
		AirspaceConverter::LogError("Expected to find at least one AIRPORT tag inside WAYPOINTS tag.");
		return false;
	}
	AirspaceConverter::LogMessage(boost::str(boost::format("This openAIP waypoint file contains %1d airfields") %numOfAirports));

	for (ptree::value_type const& ap : airportsNode) { // for all children of WAYPOINTS tag
		try {
			if (ap.first != "AIRPORT") continue;
			const ptree& airportNode(ap.second);

			// Airfield type
			std::string dataStr;
			if (!ParseAttribute(airportNode, "TYPE", dataStr)) continue; // skip not valid AIRPORT tags and TYPE attributes
			std::stringstream comments;
			int style(Waypoint::airfieldSolid); // deafult style
			switch (dataStr.at(0)) {
				case 'A':
					if (dataStr.compare("AF_CIVIL") == 0) comments << "Civil Airfield";
					else if (dataStr.compare("AF_MIL_CIVIL") == 0) comments << "Civil and Military Airport";
					else if (dataStr.compare("APT") == 0) comments << "Airport resp. Airfield IFR";
					else if (dataStr.compare("AD_CLOSED") == 0) comments << "CLOSED Airport";
					else if (dataStr.compare("AD_MIL") == 0) comments << "Military Airport";
					else if (dataStr.compare("AF_WATER") == 0) {
						style = Waypoint::airfieldGrass;
						comments << "Waterfield";
					} else continue;
					break;
				case 'G':
					if (dataStr.compare("GLIDING") == 0) {
						style = Waypoint::gliderSite;
						comments << "Glider site";
					} else continue;
					break;
				case 'H':
					if (dataStr.compare("HELI_CIVIL") == 0) comments << "Civil Heliport";
					else if (dataStr.compare("HELI_MIL") == 0) comments << "Military Heliport";
					else continue;
					break;
				case 'I':
					if (dataStr.compare("INTL_APT") == 0) comments << "International Airport";
					break;
				case 'L':
					if (dataStr.compare("LIGHT_AIRCRAFT") == 0) {
						style = Waypoint::airfieldGrass;
						comments << "Ultralight site";
					} else continue;
					break;
				default:
					continue;
			}

			// Country
			std::string countryCode;
			ParseContent(airportNode, "COUNTRY", countryCode);

			// Name
			std::string longName;
			if (!ParseContent(airportNode, "NAME", longName)) continue;

			// ICAO code
			std::string shortName;
			ParseContent(airportNode, "ICAO", shortName);

			// Geolocation
			double lat, lon, alt;
			if (!ParseGeolocation(airportNode, lat, lon, alt)) continue;

			// Runways: take the longest one
			int rwyDir(0), rwyLen(0);
			int maxstyle(Waypoint::airfieldGrass);

			// For each runway...
			comments << std::fixed;
			for (ptree::value_type const& rwy : airportNode) {
				if (rwy.first != "RWY") continue;
				const ptree& runwyNode(rwy.second);

				// Consider only active runways
				if (!ParseAttribute(runwyNode, "OPERATIONS", dataStr) || dataStr.compare("ACTIVE") != 0) continue;

				// Get runway name
				std::string rwyName;
				if (!ParseContent(runwyNode, "NAME", rwyName)) continue;

				// Get surface type
				std::string surface;
				if (!ParseContent(runwyNode, "SFC", surface)) continue;
				const int rwyStyle = !surface.empty() && (surface.at(0) == 'A' || surface.at(0) == 'C') ? Waypoint::airfieldSolid : Waypoint::airfieldGrass; // Default grass

				// Runway length
				double length = 0;
				if (!ParseMeasurement(runwyNode, "LENGTH", 'M', length)) continue;

				// Runway direction
				const ptree& dirNode(runwyNode.get_child("DIRECTION"));
				if (!ParseAttribute(dirNode, "TC", dataStr)) continue;
				double dir = std::stod(dataStr);

				// Add runway to comments
				comments << ", " << rwyName << ' ' << surface << ' ' << std::setprecision(0) << length << "m " << std::setw(3) << std::setfill('0') << dir;

				// Check if we found the longest one
				if (length > rwyLen) {
					rwyLen = (int)std::round(length);
					rwyDir = (int)std::round(dir);
					maxstyle = rwyStyle;
				}
			} // for each runway

			if (rwyLen > 0 && style != Waypoint::gliderSite) style = maxstyle; //if is not already a gliding site we just check if is "solid" surface or not...

			//Radio frequencies: if more than one just take the first "communication"
			int freqHz(0), secondaryFreqHz(0);
			if (airportNode.count("RADIO") > 0) {
				comments << std::setprecision(3);
				for (ptree::value_type const& radio : airportNode) {
					if (radio.first != "RADIO") continue;
					const ptree& radioNode(radio.second);
					std::string type;
					if (ParseAttribute(radioNode, "CATEGORY", dataStr) && ParseContent(radioNode, "TYPE", type)) {
						double frequencyMHz;
						if (!ParseValue(radioNode, "FREQUENCY", frequencyMHz)) continue;
						int frequencyHz;
						if (AirspaceConverter::CheckAirbandFrequency(frequencyMHz,frequencyHz)) switch (dataStr.at(0)) {
							case 'C': //COMMUNICATION Frequency used for communication
								if (freqHz == 0) freqHz = frequencyHz;
								else if (secondaryFreqHz == 0) secondaryFreqHz = frequencyHz;
								/* no break */
							case 'I': //INFORMATION Frequency to automated information service
							case 'N': //NAVIGATION Frequency used for navigation
							case 'O': //OHER Other frequency purpose
								comments << ", " << type << " " << frequencyMHz << " MHz";
								break;
							default:
								continue;
						}
					}
				}
			}

			// Build and store the airfield
			Airfield* airfield = new Airfield(longName, shortName, countryCode, lat, lon, (float)alt, style, rwyDir, rwyLen, freqHz, comments.str());
			if (secondaryFreqHz != 0) airfield->SetOtherFrequency(secondaryFreqHz);
			waypoints.insert(std::pair<int, Waypoint*>(style, (Waypoint*)airfield));
		} catch(...) {
			AirspaceConverter::LogError("Exception while reading openAIP airports: airfield skipped");
		}
	}
	return true;
}

bool OpenAIP::ParseNavAids(const ptree& navAidsNode) {
	int numOfNavAids = navAidsNode.count("NAVAID"); //count number of navaids in the file
	if (numOfNavAids < 1) {
		AirspaceConverter::LogError("Expected to find at least one NAVAID tag inside NAVAIDS tag.");
		return false;
	}
	AirspaceConverter::LogMessage(boost::str(boost::format("This openAIP navaids file contains %1d navigation aids") %numOfNavAids));

	for (ptree::value_type const& na : navAidsNode) { // for all children of WAYPOINTS tag
		try {
			if (na.first != "NAVAID") continue;
			const ptree& navAidNode(na.second);

			// Skip not valid NAVAID tags and TYPE attributes
			std::string dataStr;
			if (!ParseAttribute(navAidNode, "TYPE", dataStr)) continue;

			// Waypoint type
			int style(Waypoint::unknown); // deafult style
			switch (dataStr.at(0)) {
				case 'D':
					if (dataStr.compare("DME") == 0 || dataStr.compare("DVOR") == 0 || dataStr.compare("DVOR-DME") == 0 || dataStr.compare("DVORTAC") == 0) style = Waypoint::VOR;
					break;
				case 'N':
					if (dataStr.compare("NDB") == 0) style = Waypoint::NDB;
					break;
				case 'V':
					if (dataStr.compare("VOR") == 0 || dataStr.compare("VOR-DME") == 0 || dataStr.compare("VORTAC") == 0) style = Waypoint::VOR;
					break;
				case 'T':
					if (dataStr.compare("TACAN") == 0) style = Waypoint::VOR;
					break;
				default:
					continue; // skip unknown waypoints
			}
			if (style == Waypoint::unknown) continue; // skip unknown waypoints

			// Write down in the comments what it is
			std::stringstream comments;
			comments << dataStr;

			// Country
			std::string countryCode;
			ParseContent(navAidNode, "COUNTRY", countryCode);

			// Name
			std::string longName;
			if (!ParseContent(navAidNode, "NAME", longName)) continue;

			// ID code
			std::string shortName;
			ParseContent(navAidNode, "ID", shortName);

			// Geolocation
			double lat, lon, alt;
			if (!ParseGeolocation(navAidNode, lat, lon, alt)) continue;

			//Radio frequency
			int freqHz(0);
			if(navAidNode.count("RADIO") > 0) {
				const ptree& radioNode = navAidNode.get_child("RADIO");
				double freq(0);
				if (ParseValue(radioNode, "FREQUENCY", freq)) {
					if ((style == Waypoint::VOR && AirspaceConverter::CheckVORfrequency(freq,freqHz)) || (style == Waypoint::NDB && AirspaceConverter::CheckNDBfrequency(freq,freqHz)))
						comments << ", Frequency: " << std::fixed << std::setprecision(style != Waypoint::NDB ? 2 : 1) << freq << (style != Waypoint::NDB ? " MHz" : " kHz");
					else AirspaceConverter::LogWarning("skipping not valid frequency for VOR or DME for navaid: " + longName);
				}
				if (ParseContent(radioNode, "CHANNEL", dataStr)) comments << ", Channel: " << dataStr;
			}

			// Parameters
			if(navAidNode.count("PARAMS") > 0) {
				const ptree& paramsNode = navAidNode.get_child("PARAMS");
				double value(0);
				if (ParseValue(paramsNode, "RANGE", value)) comments << ", Range: " << std::fixed << std::setprecision(0) << value << " NM";
				if (ParseValue(paramsNode, "DECLINATION", value)) comments << ", Declination: " << std::setprecision(2) << value << " deg";
				if (ParseContent(paramsNode, "ALIGNEDTOTRUENORTH", dataStr)) {
					if (dataStr.compare("TRUE") == 0) comments << " true";
					else if (dataStr.compare("FALSE") == 0) comments << " magnetic";
				}
			}

			// Build and store the waypoint
			Waypoint* waypoint = new Waypoint(longName, shortName, countryCode, lat, lon, (float)alt, style, comments.str());
			if (freqHz > 0) waypoint->SetOtherFrequency(freqHz);
			waypoints.insert(std::pair<int, Waypoint*>(style, waypoint));
		} catch(...) {
			AirspaceConverter::LogError("Exception while reading openAIP navaids: waypoint skipped");
		}
	}
	return true;
}

//TODO: bool ParseHotSpots(const ptree& hotSpotsNode) {
//	return false;
//}

bool OpenAIP::ParseGeolocation(const ptree& parentNode, double &lat, double &lon, double &alt) {
	try {
		ptree node = parentNode.get_child("GEOLOCATION");
		if (!ParseValue(node,"LAT",lat) || lat < -90 || lat > 90) return false;
		if (!ParseValue(node,"LON",lon) || lon < -180 || lon > 180) return false;
		if (!ParseMeasurement(node,"ELEV",'M',alt)) return false;
		return true;
	} catch(...) {
		AirspaceConverter::LogError("Unable to parse GEOLOCATION tag");
	}
	return false;
}

bool OpenAIP::ParseContent(const ptree& parentNode, const std::string& tagName, std::string& outputString) {
	try {
		outputString = parentNode.get<std::string>(tagName);
		return true;
	} catch(...) {}
	return false;
}

bool OpenAIP::ParseAttribute(const ptree& node, const std::string& attributeName, std::string& outputString) {
	try {
		outputString = node.get_child("<xmlattr>").get<std::string>(attributeName);
		return true;
	} catch(...) {
		AirspaceConverter::LogError("Unable to parse attribute: " + attributeName);
	}
	return false;
}

bool OpenAIP::ParseValue(const ptree& parentNode, const std::string& tagName, double &value) {
	std::string dataStr;
	if (ParseContent(parentNode,tagName,dataStr)) {
		if (dataStr.empty()) {
			AirspaceConverter::LogWarning("Empty content found while expecting a numerical value from tag: " + tagName);
			return false;
		}
		try {
				value = std::stod(dataStr);
				return true;
		} catch(...) {
			AirspaceConverter::LogError("Unable to parse numerical value from tag: " + tagName);
		}
	}
	return false;
}

bool OpenAIP::ParseMeasurement(const ptree& parentNode, const std::string& tagName, char expectedUnit, double &value) {
	try {
		ptree node = parentNode.get_child(tagName);
		std::string dataStr;
		if (ParseAttribute(node,"UNIT",dataStr) && dataStr.length() == 1 && dataStr.at(0) == expectedUnit) return ParseValue(parentNode,tagName,value);
		else AirspaceConverter::LogError("Expected measure unit not found for tag: " + tagName);
	} catch(...) {
		AirspaceConverter::LogError("Unable to parse tag: " + tagName);
	}
	return false;
}
