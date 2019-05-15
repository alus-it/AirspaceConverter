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
#include <fstream>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional/optional.hpp>
#include <boost/tokenizer.hpp>

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
	/*std::ifstream input(fileName);
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
		//boost::optional<const ptree&>
		boost::optional<ptree> node = root.get_child_optional("WAYPOINTS");
		if (node) wptFound=ParseAirports((ptree&)node);

		// Look for the 'root' of navigation aids: NAVAIDS tag
		node = root.get_child_optional("NAVAIDS");
		if (node) wptFound = wptFound || ParseNavAids(node);

		// Look for the 'root' of hot spots: HOTSPOTS tag
		node = root.get_child_optional("HOTSPOTS");
		if (node) AirspaceConverter::LogWarning("openAIP hotspots not parsed because not supported yet."); //TODO: wptFound = wptFound || ParseHotSpots(node);
	} catch (...) {
		AirspaceConverter::LogError("Exception while parsing openAIP waypoints file: " + fileName);
		assert(false);
		return false;
	}
	if(!wptFound) AirspaceConverter::LogWarning("Waypoints of any kind not found in this OpenAIP file: " + fileName);
	return wptFound;*/
	return false;
}

bool ParseAirports(const ptree& airportsNode) {
	/*int numOfAirports = airportsNode.nChildNode(TEXT("AIRPORT")); //count number of airports in the file
	if (numOfAirports < 1) {
		AirspaceConverter::LogError("Expected to find at least one AIRPORT tag inside WAYPOINTS tag.");
		return false;
	}
	if (numOfAirports != airportsNode.nChildNode()) {
		AirspaceConverter::LogError("Expected to find only AIRPORT tags inside WAYPOINTS tag.");
		return false;
	} else AirspaceConverter::LogMessage("openAIP number of airports found: " + numOfAirports);

	bool return_success = true;
	XMLNode AirportNode;
	LPCTSTR dataStr = nullptr;
	for (int i = 0; i < numOfAirports && return_success; i++) {
		AirportNode = airportsNode.getChildNode(i);

		// Skip not valid AIRPORT tags and TYPE attributes
		if (!GetAttribute(AirportNode, TEXT("TYPE"), dataStr))
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
		new_waypoint.Style = STYLE_AIRFIELDSOLID; // default style: solid
		new_waypoint.Flags = AIRPORT + LANDPOINT;

		std::basic_stringstream<TCHAR> comments;

		switch (dataStr[0]) {
		case 'A':
			if (_tcsicmp(dataStr, _T("AF_CIVIL")) == 0)
				comments << "Civil Airfield" << std::endl;
			else if (_tcsicmp(dataStr, _T("AF_MIL_CIVIL")) == 0)
				comments << "Civil and Military Airport" << std::endl;
			else if (_tcsicmp(dataStr, _T("APT")) == 0)
				comments << "Airport resp. Airfield IFR" << std::endl;
			else if (_tcsicmp(dataStr, _T("AD_CLOSED")) == 0)
				comments << "CLOSED Airport" << std::endl;
			else if (_tcsicmp(dataStr, _T("AD_MIL")) == 0)
				comments << "Military Airport" << std::endl;
			else if (_tcsicmp(dataStr, _T("AF_WATER")) == 0) {
				new_waypoint.Style = STYLE_AIRFIELDGRASS;
				comments << "Waterfield" << std::endl;
			}
			break;
		case 'G':
			if (_tcsicmp(dataStr, _T("GLIDING")) == 0) {
				new_waypoint.Style = STYLE_GLIDERSITE;
				comments << "Glider site" << std::endl;
			}
			break;
		case 'H':
			if (!ISGAAIRCRAFT)
				continue; // Consider heliports only for GA aircraft
			if (_tcsicmp(dataStr, _T("HELI_CIVIL")) == 0) {
				new_waypoint.Style = STYLE_AIRFIELDSOLID;
				comments << "Civil Heliport" << std::endl;
			} else if (_tcsicmp(dataStr, _T("HELI_MIL")) == 0) {
				new_waypoint.Style = STYLE_AIRFIELDSOLID;
				comments << "Military Heliport" << std::endl;
			}
			break;
		case 'I':
			if (_tcsicmp(dataStr, _T("INTL_APT")) == 0)
				comments << "International Airport" << std::endl;
			break;
		case 'L':
			if (_tcsicmp(dataStr, _T("LIGHT_AIRCRAFT")) == 0) {
				new_waypoint.Style = STYLE_AIRFIELDGRASS;
				comments << "Ultralight site" << std::endl;
			}
			break;
		default:
			continue;
		}
		// Skip unknown waypoints
		if (new_waypoint.Style == -1)
			continue;

		// Country
		if (GetContent(AirportNode, TEXT("COUNTRY"), dataStr)) {
			LK_tcsncpy(new_waypoint.Country, dataStr, CUPSIZE_COUNTRY);
			if (_tcslen(dataStr) > 3)
				new_waypoint.Country[3] = _T('\0');
		}

		// Name
		if (GetContent(AirportNode, TEXT("NAME"), dataStr)) {
			CopyTruncateString(new_waypoint.Name, NAME_SIZE, dataStr);
		} else
			continue;

		// ICAO code
		if (GetContent(AirportNode, TEXT("ICAO"), dataStr)) {
			LK_tcsncpy(new_waypoint.Code, dataStr, CUPSIZE_CODE);
			if (_tcslen(dataStr) > CUPSIZE_CODE)
				new_waypoint.Code[CUPSIZE_CODE] = _T('\0');
		}

		// Geolocation
		if (!GetGeolocation(AirportNode, new_waypoint.Latitude,
				new_waypoint.Longitude, new_waypoint.Altitude))
			continue;

		//Radio frequencies: if more than one just take the first "communication"
		int numOfNodes = AirportNode.nChildNode(TEXT("RADIO"));
		XMLNode node, subNode;
		bool found = false, toWrite(numOfNodes == 1);
		for (int j = 0; j < numOfNodes; j++) {
			node = AirportNode.getChildNode(TEXT("RADIO"), j);
			LPCTSTR type = nullptr;
			if (GetAttribute(node, TEXT("CATEGORY"), dataStr)
					&& GetContent(node, TEXT("TYPE"), type)) {
				LPCTSTR freq = nullptr;
				if (!GetContent(node, TEXT("FREQUENCY"), freq))
					continue;
				switch (dataStr[0]) { //AlphaLima
				case 'C': //COMMUNICATION Frequency used for communication
					comments << type << " " << new_waypoint.Name << " " << freq
							<< " MHz " << std::endl;
					if (!found)
						toWrite = true;
					break;
				case 'I': //INFORMATION Frequency to automated information service
					comments << type << " " << new_waypoint.Name << " " << freq
							<< " MHz " << std::endl;
					break;
				case 'N': //NAVIGATION Frequency used for navigation
					comments << type << " " << new_waypoint.Name << " " << freq
							<< " MHz " << std::endl;
					break;
				case 'O': //OHER Other frequency purpose
					comments << type << " " << new_waypoint.Name << " " << freq
							<< " MHz " << std::endl;
					break;
				default:
					continue;
				}
				if (toWrite) {
					LK_tcsncpy(new_waypoint.Freq, freq, CUPSIZE_FREQ);
					if (_tcslen(freq) > CUPSIZE_FREQ)
						new_waypoint.Freq[CUPSIZE_FREQ] = _T('\0');
					toWrite = false;
					found = true;
				}
			}
		}

		// Runways: take the longest one
		double maxlength = 0, maxdir = 0;
		short maxstyle = STYLE_AIRFIELDGRASS;

		// For each runway...
		numOfNodes = AirportNode.nChildNode(TEXT("RWY"));
		for (int k = 0; k < numOfNodes; k++) {
			node = AirportNode.getChildNode(TEXT("RWY"), k);

			// Consider only active runways
			if (!GetAttribute(node, TEXT("OPERATIONS"), dataStr)
					|| _tcsicmp(dataStr, _T("ACTIVE")) != 0)
				continue;

			// Get runway name
			LPCTSTR name = nullptr;
			if (!GetContent(node, TEXT("NAME"), name))
				continue;

			// Get surface type
			LPCTSTR surface = nullptr;
			if (!GetContent(node, TEXT("SFC"), surface))
				continue;
			short style =
					surface[0] == 'A' || surface[0] == 'C' ?
							STYLE_AIRFIELDSOLID : STYLE_AIRFIELDGRASS; // Default grass
//            switch(surface[0]) {
//            case 'A': // ASPH Asphalt
//                style=STYLE_AIRFIELDSOLID;
//                break;
//            case 'C': // CONC Concrete
//                style=STYLE_AIRFIELDSOLID;
//                break;
//            case 'G': //GRAS Grass GRVL Gravel
//                break;
//            case 'I': // ICE
//                break;
//            case 'S': //SAND SNOW SOIL
//                break;
//            case 'U': //UNKN Unknown
//                break;
//            case 'W': //WATE Water
//                break;
//            default:
//                continue;
//            }

			// Runway length
			double length = 0;
			if (!GetMeasurement(node, TEXT("LENGTH"), 'M', length))
				continue;

			// Runway direction
			subNode = node.getChildNode(TEXT("DIRECTION"), 0);
			if (!GetAttribute(subNode, TEXT("TC"), dataStr))
				continue;
			double dir = _tcstod(dataStr, nullptr);

			// Add runway to comments
			comments << name << " " << surface << " " << length << "m " << dir
					<< MsgToken(2179) << std::endl;

			// Check if we found the longest one
			if (length > maxlength) {
				maxlength = length;
				maxdir = dir;
				maxstyle = style;
			}
		} // for each runway

		if (maxlength > 0) {
			new_waypoint.RunwayLen = maxlength;
			new_waypoint.RunwayDir = maxdir;
			if (new_waypoint.Style != STYLE_GLIDERSITE)
				new_waypoint.Style = maxstyle; //if is not already a gliding site we just check if is "solid" surface or not...
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
				// ownership of this 2 pointer has benn transfered to WaypointList
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
	}
	return return_success;*/
	return false;
}

bool ParseNavAids(const ptree& navAidsNode) {
	/*int numOfNavAids = navAidsNode.nChildNode(TEXT("NAVAID")); //count number of navaids in the file
	if (numOfNavAids < 1) {
		StartupStore(
				TEXT(
						".. Expected to find at least one NAVAID tag inside NAVAIDS tag.%s"),
				NEWLINE);
		return false;
	}
	if (numOfNavAids != navAidsNode.nChildNode()) {
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
	/*ptree node = parentNode.get_child("GEOLOCATION");
	if (!node.isEmpty()) {
		if (!GetValue(node, TEXT("LAT"), lat) || lat < -90 || lat > 90)
			return false;
		if (!GetValue(node, TEXT("LON"), lon) || lon < -180 || lon > 180)
			return false;
		if (!GetMeasurement(node, TEXT("ELEV"), 'M', alt))
			return false;
		return true;
	}*/
	return false;
}

bool OpenAIP::ParseContent(const ptree& parentNode, const std::string& tagName, std::string& outputString) {
	/*XMLNode node = parentNode.getChildNode(tagName);
	return (!node.isEmpty() && (outputString = node.getText(0)) != nullptr && outputString[0] != '\0');*/
	return false;
}

bool OpenAIP::ParseAttribute(const ptree& node, const std::string& attributeName, std::string& outputString) {
	//return (!node.isEmpty() && (outputString = node.getAttribute(attributeName)) != nullptr && outputString[0] != '\0');
	return false;
}

bool OpenAIP::ParseValue(const ptree& parentNode, const std::string& tagName, double &value) {
	/*std::string dataStr;
	if (GetContent(parentNode, tagName, dataStr)) {
		value = _tcstod(dataStr, nullptr);
		return true;
	}*/
	return false;
}

bool OpenAIP::ParseMeasurement(const ptree& parentNode, const std::string& tagName, char expectedUnit, double &value) {
	/*XMLNode node = parentNode.getChildNode(tagName);
	std::string dataStrr;
	if (GetAttribute(node, TEXT("UNIT"), dataStr) && _tcslen(dataStr) == 1
			&& dataStr[0] == expectedUnit
			&& (dataStr = node.getText(0)) != nullptr && dataStr[0] != '\0') {
		value = _tcstod(dataStr, nullptr);
		return true;
	}*/
	return false;
}
