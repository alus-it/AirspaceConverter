//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "OpenAIP.h"
#include "Airspace.h"
#include "AirspaceConverter.h"
#include "Waypoint.h"
#include "Airfield.h"
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
		AirspaceConverter::LogError("Unable to open openAIP waypoints input file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Reading openAIP waypoints file: " + fileName);
	ptree tree;
	read_xml(input, tree);
	input.close();
	ptree root;
	bool wptFound = false;
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
		if(root.count("HOTSPOTS") > 0) AirspaceConverter::LogWarning("openAIP hotspots not parsed because not supported yet."); //TODO: wptFound = wptFound || ParseNavAids(root.get_child("NAVAIDS"));
	} catch (...) {
		AirspaceConverter::LogError("Exception while parsing openAIP waypoints file: " + fileName);
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
	AirspaceConverter::LogMessage(boost::str(boost::format("Opening openAIP waypoint file number of airports found: %1d") %numOfAirports));

	std::string dataStr, longName, shortName, countryCode;
	double lat, lon, alt;
	float freq, secondaryFreq;
	int style, rwyDir, rwyLen;
	std::stringstream comments;

	try {
		for (ptree::value_type const& ap : airportsNode) { // for all children of WAYPOINTS tag
			if (ap.first != "AIRPORT") continue;
			const ptree& airportNode(ap.second);

			// Airfield type
			if (!ParseAttribute(airportNode, "TYPE", dataStr)) continue; // skip not valid AIRPORT tags and TYPE attributes
			style = Waypoint::airfieldSolid; // deafult style
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
			ParseContent(airportNode, "COUNTRY", countryCode);

			// Name
			if (!ParseContent(airportNode, "NAME", longName)) continue;

			// ICAO code
			ParseContent(airportNode, "ICAO", shortName);

			// Geolocation
			if (!ParseGeolocation(airportNode, lat, lon, alt)) continue;

			// Runways: take the longest one
			rwyDir=0;
			rwyLen=0;
			int maxstyle = Waypoint::airfieldGrass;

			// For each runway...
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
				int rwyStyle = surface.at(0) == 'A' || surface.at(0) == 'C' ? Waypoint::airfieldSolid : Waypoint::airfieldGrass; // Default grass

				// Runway length
				double length = 0;
				if (!ParseMeasurement(runwyNode, "LENGTH", 'M', length)) continue;

				// Runway direction
				const ptree& dirNode(runwyNode.get_child("DIRECTION"));
				if (!ParseAttribute(dirNode, "TC", dataStr)) continue;
				double dir = std::stod(dataStr);

				// Add runway to comments
				comments << rwyName << " " << surface << " " << length << "m " << std::setw(3) << std::setfill('0') << dir;

				// Check if we found the longest one
				if (length > rwyLen) {
					rwyLen = length;
					rwyDir = dir;
					maxstyle = rwyStyle;
				}
			} // for each runway

			if (rwyLen > 0 && style != Waypoint::gliderSite) style = maxstyle; //if is not already a gliding site we just check if is "solid" surface or not...

			//Radio frequencies: if more than one just take the first "communication"

			const int numOfFreq = airportNode.count("RADIO");
			freq = 0;
			secondaryFreq = 0;
			if (numOfFreq > 0) for (ptree::value_type const& radio : airportNode) {
				if (radio.first != "RADIO") continue;
				const ptree& radioNode(radio.second);
				std::string type;
				if (ParseAttribute(radioNode, "CATEGORY", dataStr) && ParseContent(radioNode, "TYPE", type)) {
					double frequency;
					if (!ParseValue(radioNode, "FREQUENCY", frequency)) continue;
					if (AirspaceConverter::IsValidAirbandFrequency((float)frequency)) switch (dataStr.at(0)) {
						case 'C': //COMMUNICATION Frequency used for communication
							if (freq == 0) freq = (float)frequency;
							else if (secondaryFreq == 0) secondaryFreq = (float)frequency;
							/* no break */
						case 'I': //INFORMATION Frequency to automated information service
						case 'N': //NAVIGATION Frequency used for navigation
						case 'O': //OHER Other frequency purpose
							comments << ", " << type << " " << std::fixed << std::setprecision(3) << frequency << " MHz";
							break;
						default:
							continue;
					}
				}
			}

			Airfield* airfield = new Airfield(longName, shortName, countryCode, lat, lon, (float)alt, style, rwyDir, rwyLen, freq, comments.str());
			if (AirspaceConverter::IsValidAirbandFrequency(secondaryFreq)) airfield->SetOtherFrequency(secondaryFreq);
			waypoints.insert(std::pair<int, Waypoint*>(style, (Waypoint*)airfield));
		}
		return true;
	} catch(...) {
		AirspaceConverter::LogError("Exception while reading openAIP airports file");
	}

	return false;
}

bool OpenAIP::ParseNavAids(const ptree& navAidsNode) {
	int numOfNavAids = navAidsNode.count("NAVAID"); //count number of navaids in the file
	if (numOfNavAids < 1) {
		AirspaceConverter::LogError("Expected to find at least one NAVAID tag inside NAVAIDS tag.");
		return false;
	}
	/*if (numOfNavAids != navAidsNode.nChildNode()) {
		StartupStore(
				TEXT(
						".. Expected to find only NAVAID tags inside NAVAIDS tag.%s"),
				NEWLINE);
		return false;
	} else
		StartupStore(TEXT(".. OpenAIP nav aids file contains: %u nav aids.%s"),
				(unsigned) numOfNavAids, NEWLINE);

	XMLNode NavAidNode;
	LPCTSTR dataStr = nullptr;

	bool return_success = true;

	for (int i = 0; i < numOfNavAids && return_success; i++) {
		NavAidNode = navAidsNode.getChildNode(i);

		// Skip not valid NAVAID tags and TYPE attributes
		if (!GetAttribute(NavAidNode, TEXT("TYPE"), dataStr))
			continue;

		// Prepare the new waypoint
		WAYPOINT new_waypoint;
		new_waypoint.Details = nullptr;
		new_waypoint.Comment = nullptr;
		new_waypoint.Visible = true; // default all waypoints visible at start
		new_waypoint.FarVisible = true;
		new_waypoint.Format = LKW_OPENAIP;
		new_waypoint.Number = WayPointList.size();
		new_waypoint.FileNum = globalFileNum;
		new_waypoint.Style = STYLE_NORMAL; // default style: normal

		std::basic_stringstream<TCHAR> comments;

		switch (dataStr[0]) {
		case 'D': //STYLE_VOR //
			if (_tcsicmp(dataStr, _T("DME")) == 0
					|| _tcsicmp(dataStr, _T("DVOR")) == 0
					|| _tcsicmp(dataStr, _T("DVOR-DME")) == 0
					|| _tcsicmp(dataStr, _T("DVORTAC")) == 0)
				new_waypoint.Style = STYLE_VOR;
			break;
		case 'N':
			if (_tcsicmp(dataStr, _T("NDB")) == 0)
				new_waypoint.Style = STYLE_NDB;
			break;
		case 'V':
			if (_tcsicmp(dataStr, _T("VOR")) == 0
					|| _tcsicmp(dataStr, _T("VOR-DME")) == 0
					|| _tcsicmp(dataStr, _T("VORTAC")) == 0)
				new_waypoint.Style = STYLE_VOR;
			break;
		case 'T':
			if (_tcsicmp(dataStr, _T("TACAN")) == 0)
				new_waypoint.Style = STYLE_VOR;
			break;
		default:
			continue;
		}
		// Skip unknown waypoints
		if (new_waypoint.Style == STYLE_NORMAL)
			continue;

		// Write down in the comments what it is
		comments << dataStr << std::endl;

		// Country
		if (GetContent(NavAidNode, TEXT("COUNTRY"), dataStr)) {
			LK_tcsncpy(new_waypoint.Country, dataStr, CUPSIZE_COUNTRY);
			if (_tcslen(dataStr) > 3)
				new_waypoint.Country[3] = _T('\0');
		}

		// Name
		if (GetContent(NavAidNode, TEXT("NAME"), dataStr)) {
			CopyTruncateString(new_waypoint.Name, NAME_SIZE, dataStr);
		} else
			continue;

		// Navigational aid ID
		if (GetContent(NavAidNode, TEXT("ID"), dataStr)) {
			LK_tcsncpy(new_waypoint.Code, dataStr, CUPSIZE_CODE);
			if (_tcslen(dataStr) > CUPSIZE_CODE)
				new_waypoint.Code[CUPSIZE_CODE] = _T('\0');
		}

		// Geolocation
		if (!GetGeolocation(NavAidNode, new_waypoint.Latitude,
				new_waypoint.Longitude, new_waypoint.Altitude))
			continue;

		//Radio frequency
		XMLNode node = NavAidNode.getChildNode(TEXT("RADIO"));
		if (node.isEmpty())
			continue;
		if (!GetContent(node, TEXT("FREQUENCY"), dataStr))
			continue;
		comments << "Frequency: " << dataStr << " MHz";
		LK_tcsncpy(new_waypoint.Freq, dataStr, CUPSIZE_FREQ);
		if (_tcslen(dataStr) > CUPSIZE_FREQ)
			new_waypoint.Freq[CUPSIZE_FREQ] = _T('\0');
		if (GetContent(node, TEXT("CHANNEL"), dataStr))
			comments << " Channel: " << dataStr;
		comments << std::endl;

		// Parameters
		node = NavAidNode.getChildNode(TEXT("PARAMS"));
		if (node.isEmpty())
			continue;
		double value = 0;
		if (GetValue(node, TEXT("RANGE"), value))
			comments << "Range: " << value << " NM ";
		if (GetValue(node, TEXT("DECLINATION"), value))
			comments << "Declination: " << value << MsgToken(2179);
		if (GetContent(node, TEXT("ALIGNEDTOTRUENORTH"), dataStr)) {
			if (_tcsicmp(dataStr, _T("TRUE")) == 0)
				comments << " True north";
			else if (_tcsicmp(dataStr, _T("TRUE")) == 0)
				comments << " Magnetic north";
		}

		// Add the comments
		std::basic_string<TCHAR> str(comments.str());
		new_waypoint.Comment = (TCHAR*) malloc(
				(str.length() + 1) * sizeof(TCHAR));
		if (new_waypoint.Comment != nullptr) {
			std::copy(str.begin(), str.end(), new_waypoint.Comment);
			new_waypoint.Comment[str.length()] = '\0';
		}

		// Add the new waypoint
		if (WaypointInTerrainRange(&new_waypoint)) {
			if (AddWaypoint(new_waypoint)) {
				// ownership of this 2 pointer has been transfered to WaypointList
				new_waypoint.Details = nullptr;
				new_waypoint.Comment = nullptr;
			} else {
				return_success = false;
			}
		}
		free(new_waypoint.Comment);
		free(new_waypoint.Details);
		new_waypoint.Details = nullptr;
		new_waypoint.Comment = nullptr;
	} // end of for each nav aid
	return return_success;*/
	return false;
}

/* TODO:
bool ParseHotSpots(const ptree& hotSpotsNode) {
	int numOfHotSpots = hotSpotsNode.nChildNode(TEXT("HOTSPOT")); //count number of hotspots in the file
	if (numOfHotSpots < 1) {
		StartupStore(
				TEXT(
						".. Expected to find at least one HOTSPOT tag inside HOTSPOTS tag.%s"),
				NEWLINE);
		return false;
	}
	if (numOfHotSpots != hotSpotsNode.nChildNode()) {
		StartupStore(
				TEXT(
						".. Expected to find only HOTSPOT tags inside HOTSPOTS tag.%s"),
				NEWLINE);
		return false;
	} else
		StartupStore(
				TEXT(".. OpenAIP hot spots file contains: %u hot spots.%s"),
				(unsigned) numOfHotSpots, NEWLINE);

	bool return_success = true;
	XMLNode HotSpotNode;
	LPCTSTR dataStr = nullptr;
	for (int i = 0; i < numOfHotSpots && return_success; i++) {
		HotSpotNode = hotSpotsNode.getChildNode(i);

		// Skip not valid HOTSPOT tags and TYPE attributes
		if (!GetAttribute(HotSpotNode, TEXT("TYPE"), dataStr))
			continue;

		// Thermal type
		switch (dataStr[0]) {
		case 'A':
			if (_tcsicmp(dataStr, _T("ARTIFICIAL")) != 0)
				continue;
			break;
		case 'N':
			if (_tcsicmp(dataStr, _T("NATURAL")) != 0)
				continue;
			break;
		default:
			continue;
		}

		// Write type down in the comments
		std::basic_stringstream<TCHAR> comments;
		comments << dataStr;

		// Aircraftcategories: if glider ignore small thermals for paragliders
		XMLNode node = HotSpotNode.getChildNode(TEXT("AIRCRAFTCATEGORIES"));
		if (!node.isEmpty()) {
			int numOfCategories = node.nChildNode(TEXT("AIRCRAFTCATEGORY"));
			if (numOfCategories != node.nChildNode())
				continue;
			bool gliders = false, hangGliders = false; //, paraGliders=false
			for (int j = 0; j < numOfCategories; j++) {
				XMLNode subNode = node.getChildNode(j);
				if (!subNode.isEmpty()
						&& (dataStr = subNode.getText(0)) != nullptr
						&& dataStr[0] != '\0') {
					if (_tcsicmp(dataStr, _T("GLIDER")) == 0)
						gliders = true;
					else if (_tcsicmp(dataStr, _T("HANG_GLIDER")) == 0)
						hangGliders = true;
					//else if(_tcsicmp(dataStr,_T("PARAGLIDER"))==0) paraGliders=true;
				}
			}
			if ((ISGLIDER || ISGAAIRCRAFT) && !gliders && !hangGliders)
				continue;
		}

		// Prepare the new waypoint
		WAYPOINT new_waypoint;
		new_waypoint.Details = nullptr;
		new_waypoint.Comment = nullptr;
		new_waypoint.Visible = true; // default all waypoints visible at start
		new_waypoint.FarVisible = true;
		new_waypoint.Format = LKW_OPENAIP;
		new_waypoint.Number = WayPointList.size();
		new_waypoint.FileNum = globalFileNum;
		new_waypoint.Style = STYLE_THERMAL; // default style: thermal

		// Country
		if (GetContent(HotSpotNode, TEXT("COUNTRY"), dataStr)) {
			LK_tcsncpy(new_waypoint.Country, dataStr, CUPSIZE_COUNTRY);
			if (_tcslen(dataStr) > 3)
				new_waypoint.Country[3] = _T('\0');
		}

		// Name
		if (GetContent(HotSpotNode, TEXT("NAME"), dataStr)) {
			CopyTruncateString(new_waypoint.Name, NAME_SIZE, dataStr);
		} else
			continue;

		// Geolocation
		if (!GetGeolocation(HotSpotNode, new_waypoint.Latitude,
				new_waypoint.Longitude, new_waypoint.Altitude))
			continue;

		// Reliability
		double reliability = 0;
		if (!GetValue(HotSpotNode, TEXT("RELIABILITY"), reliability))
			continue;
		comments << " " << reliability * 100 << "% ";

		// Occourrence
		if (!GetContent(HotSpotNode, TEXT("OCCURRENCE"), dataStr))
			continue;
		comments << dataStr << std::endl;

		// Comment
		if (GetContent(HotSpotNode, TEXT("COMMENT"), dataStr))
			comments << dataStr;

		// Add the comments
		std::basic_string<TCHAR> str(comments.str());
		new_waypoint.Comment = (TCHAR*) malloc(
				(str.length() + 1) * sizeof(TCHAR));
		if (new_waypoint.Comment != nullptr) {
			std::copy(str.begin(), str.end(), new_waypoint.Comment);
			new_waypoint.Comment[str.length()] = '\0';
		}

		// Add the new waypoint
		if (WaypointInTerrainRange(&new_waypoint)) {
			if (AddWaypoint(new_waypoint)) {
				// ownership of this 2 pointer has been transfered to WaypointList
				new_waypoint.Details = nullptr;
				new_waypoint.Comment = nullptr;
			} else {
				return_success = false;
			}
		}
		free(new_waypoint.Comment);
		free(new_waypoint.Details);
		new_waypoint.Details = nullptr;
		new_waypoint.Comment = nullptr;
	} // end of for each nav aid
	return return_success;
}*/

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
	} catch(...) {
		AirspaceConverter::LogError("Unable to parse content of tag: " + tagName);
	}
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
