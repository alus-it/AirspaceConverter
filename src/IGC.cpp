//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2021 Valerio Messina, Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================
// IGC file format description as used by LXnav, LK8000, ...
// Are text waypoint files, with lines like the following template:
// B0906264546312N00909832EV004870000099900
// ^ can be: A, B, C, G, H, I, L, ...
// B record are: B HHMMSS DDMMcccN DDDMMcccW A HHHHH hhhhh eee ss
//               1 234567 89012345 678901234 5 67890 12345 678 90
//               0 000000 00111111 111122222 2 22223 33333 333 34
//               p time__ latitude longitude a hhBar hhGps err sats
// See tutorial : https://xp-soaring.github.io/igc_file_format/index.html
// See reference: https://xp-soaring.github.io/igc_file_format/igc_format_2008.html
// See official : https://www.fai.org/sites/default/files/igc_fr_specification_2020-11-25_with_al6.pdf

#include "IGC.h"
#include "AirspaceConverter.h"
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/locale/encoding.hpp>

#if 0
const std::unordered_map<std::string, Waypoint::Type> IGC::IGCWaypointTable = {
	{ "A", Waypoint::CLASSA },
	{ "B", Waypoint::CLASSB },
	{ "C", Waypoint::CLASSC },
	{ "D", Waypoint::CLASSD },
	{ "E", Waypoint::CLASSE },
	{ "F", Waypoint::CLASSF },
	{ "G", Waypoint::CLASSG },
	{ "Q", Waypoint::D },
	{ "P", Waypoint::P },
	{ "R", Waypoint::R },
	{ "CTR", Waypoint::CTR },
	{ "TMZ", Waypoint::TMZ },
	{ "RMZ", Waypoint::RMZ },
	{ "GSEC", Waypoint::GLIDING },
	{ "GP", Waypoint::NOGLIDER },
	{ "W", Waypoint::WAVE },
	{ "WAVE", Waypoint::WAVE },
	{ "NOTAM", Waypoint::NOTAM },
	{ "OTHER", Waypoint::OTHER },
	{ "TMA", Waypoint::TMA },
	{ "FIR", Waypoint::FIR },
	{ "UIR", Waypoint::UIR },
	{ "OTH", Waypoint::OTH },
	{ "AWY", Waypoint::AWY },
	{ "MATZ", Waypoint::MATZ },
	{ "MTMA", Waypoint::MTMA },
	{ "MTRA", Waypoint::MTRA },
	{ "T", Waypoint::TFR },
	{ "TFR", Waypoint::TFR },
	{ "ADA", Waypoint::ADA },
	{ "ADIZ", Waypoint::ADIZ },
	{ "CTA", Waypoint::CTR },
	{ "DFIR", Waypoint::DFIR },
	{ "TIZ", Waypoint::TIZ },
	{ "TIA", Waypoint::TIA },
	{ "SRZ", Waypoint::SRZ },
	{ "ATZ", Waypoint::ATZ },
	{ "FISA", Waypoint::FISA },
	{ "MBZ", Waypoint::MBZ },
	{ "ASR", Waypoint::ASR },
	{ "COMP", Waypoint::COMP },
	{ "TRZ", Waypoint::TRZ },
	{ "VFRR", Waypoint::VFRR },
	{ "RTZ", Waypoint::RTZ },
	{ "PARA", Waypoint::PARA },
	{ "LFZ", Waypoint::LFZ },
	{ "CFZ", Waypoint::CFZ },
	{ "MOA", Waypoint::MOA },
	{ "MTA", Waypoint::MTA },
	{ "TSA", Waypoint::TSA },
	{ "TRA", Waypoint::TRA },
	{ "UKN", Waypoint::UNKNOWN },
	{ "UNKNOWN", Waypoint::UNKNOWN }
};

bool IGC::calculateArcs = true;
IGC::CoordinateType IGC::coordinateType = IGC::CoordinateType::AUTO;*/
#endif

IGC::IGC(std::multimap<int,Waypoint*>& waypointsMap):
	waypoints(waypointsMap) {
	/*waypoints(waypointsMap),
	varRotationClockwise(true),
	lastACline(-1),
	lastPointWasDDMMSS(false),
	lastLatD(Geometry::LatLon::UNDEF_LAT),
	lastLatM(0),
	lastLatS(0),
	lastLonD(Geometry::LatLon::UNDEF_LON),
	lastLonM(0),
	lastLonS(0) {*/
}

#if 0
std::string& IGC::RemoveComments(std::string &s) {
	s.erase(find_if(s.begin(), s.end(), [](const char c) { return c == '*'; }), s.end());
	return s;
}

bool IGC::ParseDegrees(const std::string& dddmmss, double& deg) {
	if(dddmmss.empty()) return false;

	// Tokenize on columns
	boost::tokenizer<boost::char_separator<char>> tokens(dddmmss, boost::char_separator<char>(":"));
	const int fields = (int)std::distance(tokens.begin(),tokens.end());
	if(fields < 1 || fields > 3) return false; // We expect from 1 to 3 fields

	// Degrees
	boost::tokenizer<boost::char_separator<char>>::iterator token=tokens.begin();
	if ((*token).empty()) return false;
	try {
		deg = std::stoi(*token);

		// Minutes
		if (++token != tokens.end()) {
			if ((*token).empty()) return false;
			deg += std::stod(*token) / 60;

			// Seconds
			if (++token != tokens.end()) {
				if ((*token).empty()) return false;
				deg += std::stod(*token) / 3600;
			}
		}
	} catch (...) {
		return false;
	}
	return true;
}

bool IGC::ParseCoordinates(const std::string& text, Geometry::LatLon& point) {
	// Tokenize on spaces
	boost::tokenizer<boost::char_separator<char> > tokens(text, boost::char_separator<char>(" "));
	if(std::distance(tokens.begin(),tokens.end()) < 2) return false; // We expect at least 2 fields

	// Latitude degrees, minutes and seconds
	boost::tokenizer<boost::char_separator<char>>::iterator token=tokens.begin();
	std::string coord(*token);
	char sign;
	if(coord.size()>1 && !AirspaceConverter::isDigit(coord.back())) {
		// The N or S is not spaced from the coordinates
		sign = coord.back();
		coord = coord.substr(0, coord.size()-1);
	} else {
		// Latitude sign N or S should be in the next token
		token++; // here we know already that there are at least two tokens
		if ((*token).length() == 1) sign = (*token).front();
		else return false;
	}

	// Parse the latitude
	double lat = Geometry::LatLon::UNDEF_LAT;
	if (!ParseDegrees(coord, lat)) return false;

	// Apply latitude sign N or S
	if (sign == 'S' || sign == 's') lat = -lat;
	else if (sign != 'N' && sign != 'n') return false;

	// Verify validity of latitude
	if (!Geometry::LatLon::IsValidLat(lat)) return false;

	// Longitude degrees, minutes and seconds
	if (++token == tokens.end()) return false;
	coord = *token;
	if(coord.size()>1 && !AirspaceConverter::isDigit(coord.back())) {
		// The E or W is not spaced from the coordinates
		sign = coord.back();
		coord = coord.substr(0, coord.size()-1);
	} else {
		// Longitude sign E or W should be in the next token
		if (++token == tokens.end()) return false;
		if ((*token).length() == 1) sign = (*token).front();
		else return false;
	}

	// Parse the longitude
	double lon = Geometry::LatLon::UNDEF_LON;
	if (!ParseDegrees(coord, lon)) return false;

	// Apply the longitude sign E or W
	if (sign == 'W' || sign == 'w') lon = -lon;
	else if (sign != 'E' && sign != 'e') return false;

	// Verify validity of longitude
	if (!Geometry::LatLon::IsValidLon(lon)) return false;

	// Finally set the point coordinates
	point.SetLatLon(lat,lon);
	return true;
}
#endif

bool IGC::IsFileUTF8(std::ifstream& inputFile) {
	if (inputFile.get() == 0xef && inputFile.get() == 0xbb && inputFile.get() == 0xbf) return true; // Check if first three characters are the UTF-8 BOM
	inputFile.seekg(0); //re-wind
	//TODO: Very few UTF-8 file has BOM, so here we should scan the file and verify if it is UTF-8
	return false;
}

#if 0
bool IGC::ParseAC(const std::string & line, Waypoint& waypoint) {
	varRotationClockwise = true; // Reset var to default at beginning of new waypoint segment
	InsertWaypoint(waypoint); // If new waypoint first store the actual one
	assert(waypoint.GetType() == Waypoint::UNDEFINED);
	Waypoint::Type type = Waypoint::UNDEFINED;
	if (line.size() < 4 || line.at(2) !=' ') return false;
	auto it = IGCWaypointTable.find(line.substr(3));
	if (it != IGCWaypointTable.end()) type = it->second;
	if (type == Waypoint::UNDEFINED) return false;
	waypoint.SetType(type);
	return true;
}

bool IGC::ParseAN(const std::string & line, Waypoint& waypoint, const bool isUTF8) {
	if (waypoint.GetType() == Waypoint::UNDEFINED) return true;
	if (line.size() < 4) return false;
	if (waypoint.GetName().empty()) {
		std::string name(line.substr(3));
		if (name == "COLORENTRY") {
			waypoint.SetType(Waypoint::UNDEFINED); // Skip Strepla colortable entries
			return true;
		}
		if (waypoint.GetType() > Waypoint::Type::OTHER && // If the type will be not (yet) recognized by LK8000 (from Type::OTHER on)
			name.rfind(waypoint.GetCategoryName()) == std::string::npos) { // ... and the name does not alredy contain it ...
			name.insert (0, waypoint.GetCategoryName() + " "); // Then make sure the name contains the type as text
		}
		waypoint.SetName(isUTF8 ? name : boost::locale::conv::between(name,"utf-8","ISO8859-1"));
		return true;
	}
	AirspaceConverter::LogError(boost::str(boost::format("waypoint %1s has already a name.") % waypoint.GetName()));
	return false;	
}

bool IGC::ParseAF(const std::string& line, Waypoint& waypoint, const bool isUTF8) {
	if (line.size() < 4) return false;
	std::string descr(line.substr(3));
	try {
		size_t pos(0);
		const double freqMHz = std::stod(descr,&pos);
		int freqHz;
		if (!AirspaceConverter::CheckAirbandFrequency(freqMHz,freqHz)) return false;
		if (pos<descr.length()) {
			descr.erase(0,pos);
			if (descr.at(0) == ' ') descr.erase(0,1); // remove the separating space
		} else descr.erase();
		waypoint.AddRadioFrequency(freqHz, isUTF8 ? descr : boost::locale::conv::between(descr,"utf-8","ISO8859-1"));
		return true;
	} catch(...) {
		return false;
	}
}

bool IGC::ParseAltitude(const std::string& line, const bool isTop, Waypoint& waypoint) {
	if (waypoint.GetType() == Waypoint::UNDEFINED) return true;
	const std::string::size_type l = line.length();
	if (l < 4) return false;
	return AirspaceConverter::ParseAltitude(line.substr(3,l-3), isTop, waypoint);
}

bool IGC::ParseS(const std::string & line) {
	if (line.size() > 1 && (line.at(1) == 'P' || line.at(1) == 'B')) return true; // ignore it...
	return false;
}

bool IGC::ParseT(const std::string& line) {
	if (line.size() > 1 && (line.at(1) == 'C' || line.at(1) == 'O')) return true; // Style, pen or brush record ignore it...
	return false;
}

bool IGC::ParseDP(const std::string& line, Waypoint& waypoint, const int& linenumber) {
	if (waypoint.GetType() == Waypoint::UNDEFINED) return true;
	if (line.length() < 14) return false;
	Geometry::LatLon point;
	if (ParseCoordinates(line.substr(3), point)) {
		if (!waypoint.AddPoint(point)) AirspaceConverter::LogWarning(boost::str(boost::format("skipping unnecessary repeated point on line %1d: %2s") % linenumber % line));
		return true;
	}
	return false;
}

bool IGC::ParseV(const std::string & line, Waypoint& waypoint) {
	if (waypoint.GetType() == Waypoint::UNDEFINED) return true;
	if (line.length() < 5) return false;
	switch (line.at(2)) {
	case 'D':
		{
			const char c = line.at(4);
			if (c == '+') varRotationClockwise = true;
			else if(c == '-') varRotationClockwise = false;
			else return false;
		}
		break;
	case 'X':
		{
			if (ParseCoordinates(line.substr(4), varPoint)) return true;
			varPoint.SetLatLon(Geometry::LatLon::UNDEF_LAT, Geometry::LatLon::UNDEF_LON);
		}
		return false;
	case 'W':
		/* try {
			varWidth = std::stod(line.substr(4));
		} catch(...) {
			varWidth = 0;
			return false;
		}*/
		break;
	case 'Z': // ignore it
		break;
	default:
		return false;
	}
	return true;
}

bool IGC::ParseDA(const std::string& line, Waypoint& waypoint) {
	if (waypoint.GetType() == Waypoint::UNDEFINED) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 8) return false;
	const std::string data(line.substr(3));
	boost::tokenizer<boost::char_separator<char>> tokens(data, boost::char_separator<char>(","));
	if (std::distance(tokens.begin(), tokens.end()) != 3) return false; // Make sure there are 3 fields
	boost::tokenizer<boost::char_separator<char>>::iterator token = tokens.begin();
	try {
		double radius = std::stod(*token);
		double angleStart = std::stod(*(++token));
		double angleEnd = std::stod(*(++token));
		waypoint.AddGeometry(new Sector(varPoint, radius, angleStart, angleEnd, varRotationClockwise));
	} catch (...) {
		return false;
	}
	return true;
}

bool IGC::ParseDB(const std::string& line, Waypoint& waypoint) {
	if (waypoint.GetType() == Waypoint::UNDEFINED) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 26) return false;
	const std::string data(line.substr(3));
	boost::tokenizer<boost::char_separator<char>> tokens(data, boost::char_separator<char>(","));
	if (std::distance(tokens.begin(), tokens.end()) != 2) return false; // Make sure there are 2 fields
	boost::tokenizer<boost::char_separator<char>>::iterator token = tokens.begin();
	Geometry::LatLon p1;
	if (!ParseCoordinates(*token, p1)) return false;
	Geometry::LatLon p2;
	if (!ParseCoordinates(*(++token), p2)) return false;
	waypoint.AddGeometry(new Sector(varPoint, p1, p2, varRotationClockwise));
	return true;
}

bool IGC::ParseDC(const std::string& line, Waypoint& waypoint) {
	if (waypoint.GetType() == Waypoint::UNDEFINED) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 4) return false;
	try {
		waypoint.AddGeometry(new Circle(varPoint, std::stod(line.substr(3))));
	} catch (...) {
		return false;
	}
	return true;
}

/* Airway not yet supported
bool IGC::ParseDY(const std::string & line, Waypoint& waypoint)
{
	if (waypoint.GetType() == Waypoint::UNDEFINED) return true;
	if (varWidth == 0 || line.length() < 14) return false;
	double lat = 0, lon = 0;
	if (ParseCoordinates(line.substr(3), lat, lon)) {
		waypoint.AddGeometry(new AirwayPoint(lat, lon, varWidth));
		return true;
	} 
	return false;
}
*/
#endif

bool IGC::ParseB(const std::string & line, Waypoint& waypoint) {
   std::string coord(line);
   char sign;
   double lat, lon;
   sign = coord.at(15);
   coord = coord.substr(8, 9);
   lat = std::stoi(coord);
   coord = coord.substr(10, 14);
   lat += std::stod(coord) / 60;
   if (sign == 'S' || sign == 's') lat = -lat;
   
   sign = coord.at(24);
   coord = coord.substr(16, 17);
   lon = std::stoi(coord);
   coord = coord.substr(18, 23);
   lon += std::stod(coord) / 60;
   if (sign == 'W' || sign == 'w') lon = -lon;
   
#if 0
	varRotationClockwise = true; // Reset var to default at beginning of new waypoint segment
	InsertWaypoint(waypoint); // If new waypoint first store the actual one
	assert(waypoint.GetType() == Waypoint::UNDEFINED);
	Waypoint::Type type = Waypoint::UNDEFINED;
	if (line.size() < 4 || line.at(2) !=' ') return false;
	auto it = IGCWaypointTable.find(line.substr(3));
	if (it != IGCWaypointTable.end()) type = it->second;
	if (type == Waypoint::UNDEFINED) return false;
	waypoint.SetType(type);
#endif
	return true;
}

bool IGC::InsertWaypoint(Waypoint& waypoint) {
	if (waypoint.GetType() == Waypoint::UNDEFINED || waypoint.GetName().empty()) {
		//waypoint.Clear();
		return false;
	}

	// Verify if is valid waypoint giving a proper explanatory error message
	bool validWaypoint/*(waypoint.GetNumberOfGeometries() > 0)*/;
#if 0
	if (!validWaypoint)
		AirspaceConverter::LogError(boost::str(boost::format("at line %1d: skip waypoint %2s with no geometries.") % lastACline % waypoint.GetName()));

	if (validWaypoint && waypoint.GetTopAltitude() <= waypoint.GetBaseAltitude()) {
		AirspaceConverter::LogError(boost::str(boost::format("at line %1d: skip waypoint %2s with top and base equal or inverted.") % lastACline % waypoint.GetName()));
		validWaypoint = false;
	}

	if (validWaypoint && waypoint.GetTopAltitude().IsGND()) {
		AirspaceConverter::LogError(boost::str(boost::format("at line %1d: skip waypoint %2s with top at GND.") % lastACline % waypoint.GetName()));
		validWaypoint = false;
	}

	// Remove repeated or very similar consecutive points
	waypoint.RemoveTooCloseConsecutivePoints();

	// Ensure that the points are closed
	if (validWaypoint && !waypoint.ClosePoints()) {
		AirspaceConverter::LogError(boost::str(boost::format("at line %1d: skip waypoint %2s with less than 3 points.") % lastACline % waypoint.GetName()));
		validWaypoint = false;
	}

	// If all OK insert the new waypoint
	if (validWaypoint) {

		// This should be just a warning
		if (waypoint.GetName().empty()) AirspaceConverter::LogWarning(boost::str(boost::format("at line %1d: waypoint without name.") % lastACline));
		
		waypoints.insert(std::pair<int, Waypoint>(waypoint.GetType(), std::move(waypoint)));
	}

	// Otherwise discard it
	else waypoint.Clear();
#endif
	return validWaypoint;	
}

// Reading and parsing IGC waypoint file
bool IGC::Read(const std::string& fileName) {
	std::ifstream input(fileName, std::ios::binary);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogError("Unable to open input file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Reading IGC file: " + fileName);

	// Check if the input file is encoded in UTF-8
	/*const*/ bool isUTF8 = IsFileUTF8(input); isUTF8 = isUTF8;

	int linecount = 0;
	std::string sLine;
	bool allParsedOK = true, isCRLF = false, CRLFwarningGiven = false;
	Waypoint waypoint;
	while (!input.eof() && input.good()) {

		// Get the line
		AirspaceConverter::SafeGetline(input, sLine, isCRLF);
		++linecount;

		// Verify line ending
		if (!CRLFwarningGiven && !isCRLF) {
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: not valid Windows style end of line (expected CR LF).") % linecount));

			// IGC files may contain thousands of lines we don't want to print this warning all the time
			CRLFwarningGiven = true;
		}
		
		// Directly skip empty lines
		if (sLine.empty()) continue;

		// Remove front spaces
		boost::algorithm::trim_left(sLine);

		// If it was a line with only spaces skip it
		if (sLine.empty()) continue;

		// Then directly skip full comment line
		//if (sLine.front() == '*') continue;

		// Remove inline comments
		//RemoveComments(sLine);

		// Remove back spaces
		boost::algorithm::trim_right(sLine);

		// Check for too short lines
		bool lineParsedOK = sLine.size() > 2;

		if (lineParsedOK) switch (sLine.at(0)) {
		case 'B':
			lineParsedOK = ParseB(sLine, waypoint);
			break;
#if 0
		case 'A':
			switch(sLine.at(1)) {
			case 'C': //AC
				lineParsedOK = ParseAC(sLine, waypoint);
				if (lineParsedOK) lastACline = linecount;
				break;
			case 'N': //AN
				lineParsedOK = ParseAN(sLine, waypoint, isUTF8);
				break;
			case 'L': //AL
				lineParsedOK = ParseAltitude(sLine, false, waypoint);
				break;
			case 'H': //AH
				lineParsedOK = ParseAltitude(sLine, true, waypoint);
				break;
			case 'F': //AF radio frequency
				lineParsedOK = ParseAF(sLine, waypoint, isUTF8);
				break;
			case 'X': //AX: transponder code
				lineParsedOK = waypoint.SetTransponderCode(sLine.substr(3));
				break;
			case 'P': //AP: (De)activation time
			case 'W': //AW: Weekly activation Time
			case 'T': //AT
			case 'G': //AG
			case 'Y': //AY // ignore all those for now...
				break;
			default:
				lineParsedOK = false;
				break;
			} // A
			break;
		case 'D':
			switch (sLine.at(1)) {
			case 'P': // DP
				lineParsedOK = ParseDP(sLine, waypoint, linecount);
				break;
			case 'A': // DA
				lineParsedOK = ParseDA(sLine, waypoint);
				break;
			case 'B': // DB
				lineParsedOK = ParseDB(sLine, waypoint);
				break;
			case 'C': // DC
				lineParsedOK = ParseDC(sLine, waypoint);
				break;
			case 'Y': // DY
				//ParseDY(sLine, waypoint); // Airway not yet supported
				AirspaceConverter::LogWarning(boost::str(boost::format("skipping airway segment (not yet supported) on line %1d: %2s") %linecount %sLine));
				lineParsedOK = false; 
				break;
			default:
				lineParsedOK = false;
				break;
			} // D
			break;
		case 'V':
			lineParsedOK = ParseV(sLine, waypoint);
			break;
		case 'S':
			lineParsedOK = ParseS(sLine);
			break;
		case 'T':
			lineParsedOK = ParseT(sLine);
			break;
#endif
		default:
			lineParsedOK = false;
			break;
		}
		if (!lineParsedOK) {
			AirspaceConverter::LogError(boost::str(boost::format("unable to parse IGC line %1d: %2s") %linecount %sLine));
			allParsedOK = false;
		}
	}
	
	// Insert last waypoint
	InsertWaypoint(waypoint);

	input.close();
	return allParsedOK;
}

#if 0
bool IGC::Write(const std::string& fileName) {
	if (waypoints.empty()) {
		AirspaceConverter::LogMessage("IGC output: no waypoint, nothing to write");
		return false;
	}
	if (file.is_open()) file.close();
	file.open(fileName, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file.is_open() || file.bad()) {
		AirspaceConverter::LogError("Unable to open output file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Writing IGC output file: " + fileName);

	WriteHeader();

	// Go trough all waypoint
	for (std::pair<const int,Waypoint>& pair : waypoints)
	{
		// Get the waypoint
		Waypoint& a = pair.second;

		// Just a couple if assertions
		assert(a.GetNumberOfPoints() > 3);
		assert(a.GetFirstPoint()==a.GetLastPoint());

		// Reset var
		varRotationClockwise = true;

		// Skip IGC not supported categories
		if (!WriteCategory(a)) continue;

		// Write the name
		file << "AN " << boost::locale::conv::between(a.GetName(),"ISO8859-1","utf-8") << "\r\n";
		
		// Write base and ceiling altitudes
		file << "AL " << a.GetBaseAltitude().ToString() << "\r\n";
		file << "AH " << a.GetTopAltitude().ToString() << "\r\n";

		// Write frequencies
		if (a.GetNumberOfRadioFrequencies() > 0) {
			file << std::fixed << std::setprecision(3);
			for (size_t i=0; i<a.GetNumberOfRadioFrequencies(); i++) {
				const std::pair<int, std::string>& f = a.GetRadioFrequencyAt(i);
				file << "AF " << AirspaceConverter::FrequencyMHz(f.first);
				if (!f.second.empty()) file << ' ' << boost::locale::conv::between(f.second,"ISO8859-1","utf-8");
				file << "\r\n";
			}
			file.unsetf(std::ios_base::floatfield); //file << std::defaultfloat; not supported by older GCC 4.9.0
		}

		// Write transponder code
		if (a.HasTransponderCode()) file << "AX " << a.GetTransponderCode() << "\r\n";

		// Set the stream
		file << std::setfill('0');

		// Write the geometries
		if (calculateArcs) {

			// Get number of geometries
			size_t numOfGeometries = a.GetNumberOfGeometries();

			// If no geometries are defined we have to calculate them
			if (numOfGeometries == 0) {
				a.Undiscretize();
				numOfGeometries = a.GetNumberOfGeometries();
			}
			assert(numOfGeometries > 0);

			// Write each geometry
			for (size_t i = 0; i < numOfGeometries; i++) a.GetGeometryAt(i)->WriteIGCGeometry(*this);
		}

		// Otherwise write every single point (except the last one which is the same)
		else for (size_t i = 0; i < a.GetNumberOfPoints() - 1; i++) WritePoint(a.GetPointAt(i));

		// Add an empty line at the end of the waypoint
		file << "\r\n";
	}
	file.close();
	return true;
}

void IGC::WriteHeader() {
	for(const std::string& line: AirspaceConverter::disclaimer) file << "* " << line << "\r\n";
	file << "\r\n* " << AirspaceConverter::GetCreationDateString() << "\r\n\r\n";
}

bool IGC::WriteCategory(const Waypoint& waypoint) {
	std::string IGCCategory;
	switch(waypoint.GetType()) {
		case Waypoint::CLASSA:		IGCCategory = "A"; break;
		case Waypoint::CLASSB:		IGCCategory = "B"; break;
		case Waypoint::CLASSC:		IGCCategory = "C"; break;
		case Waypoint::CLASSD:		IGCCategory = "D"; break;
		case Waypoint::CLASSE:		IGCCategory = "E"; break;
		case Waypoint::CLASSF:		IGCCategory = "F"; break;
		case Waypoint::CLASSG:		IGCCategory = "G"; break;
		case Waypoint::D:			IGCCategory = "Q"; break;
		case Waypoint::WAVE:		IGCCategory = "W"; break;
		case Waypoint::NOGLIDER:	IGCCategory = "GP"; break;
		case Waypoint::GLIDING:		IGCCategory = "GSEC"; break;
		case Waypoint::UNDEFINED:
			AirspaceConverter::LogWarning(boost::str(boost::format("skipping undefined waypoint %1s.") % waypoint.GetName()));
			assert(false);
			return false;
		default: IGCCategory = waypoint.CategoryName(waypoint.GetType()); break;
	}
	file << "AC " << IGCCategory << "\r\n";
	lastPointWasDDMMSS = false;
	return true;
}

void IGC::WritePoint(const Geometry::LatLon& point, bool isCenterPoint /* = false */, bool addPrefix /*= true*/) {
	if (isCenterPoint && addPrefix) file << "V X=";
	switch (coordinateType) {
		case CoordinateType::DEG_DECIMAL_MIN: {
			if (!isCenterPoint && addPrefix) file << "DP ";
			int deg;
			double decimalMin;
			point.GetLatDegMin(deg, decimalMin);
			file << deg << ":" << decimalMin << " " << point.GetNorS() << " ";
			point.GetLonDegMin(deg, decimalMin);
			file << deg << ":" << decimalMin << " " << point.GetEorW();
		}
		break;
		case CoordinateType::DEG_MIN_SEC: {
			int latD, latM, latS, lonD, lonM, lonS;
			point.GetLatDegMinSec(latD, latM, latS);
			point.GetLonDegMinSec(lonD, lonM, lonS);
			if (!isCenterPoint) {
				if (lastPointWasDDMMSS && latD == lastLatD && latM == lastLatM && latS == lastLatS && lonD == lastLonD && lonM == lastLonM && lonS == lastLonS) return;
				lastPointWasDDMMSS = true;
				lastLatD = latD;
				lastLatM = latM;
				lastLatS = latS;
				lastLonD = lonD;
				lastLonM = lonM;
				lastLonS = lonS;
			}
			if (!isCenterPoint && addPrefix) file << "DP ";
			file << std::setw(2) << latD << ":" << std::setw(2) << latM << ":" << std::setw(2) << latS << " " << point.GetNorS() << " ";
			file << std::setw(3) << lonD << ":" << std::setw(2) << lonM << ":" << std::setw(2) << lonS << " " << point.GetEorW();
		}
		break;
		case CoordinateType::AUTO:
		default: {
			int latD, latM, latS, lonD, lonM, lonS;
			double decimalLatM, decimalLonM;
			const bool latDDMMSS = point.GetAutoLatDegMinSec(latD, decimalLatM, latM, latS);
			const bool lonDDMMSS = point.GetAutoLonDegMinSec(lonD, decimalLonM, lonM, lonS);
			if (!isCenterPoint) {
				if (latDDMMSS && lonDDMMSS) {
					if (lastPointWasDDMMSS && latD == lastLatD && latM == lastLatM && latS == lastLatS && lonD == lastLonD && lonM == lastLonM && lonS == lastLonS) return;
					lastPointWasDDMMSS = true;
					lastLatD = latD;
					lastLatM = latM;
					lastLatS = latS;
					lastLonD = lonD;
					lastLonM = lonM;
					lastLonS = lonS;
				} else lastPointWasDDMMSS = false;
			}
			if (!isCenterPoint && addPrefix) file << "DP ";
			if (latDDMMSS) file << std::setw(2) << latD << ":" << std::setw(2) << latM << ":" << std::setw(2) << latS << " " << point.GetNorS() << " ";
			else file << latD << ":" << decimalLatM << " " << point.GetNorS() << " ";
			if (lonDDMMSS) file << std::setw(3) << lonD << ":" << std::setw(2) << lonM << ":" << std::setw(2) << lonS <<" " << point.GetEorW();
			else file << lonD << ":" << decimalLonM << " " << point.GetEorW();
		}
	}
	if (addPrefix) file << "\r\n";
}

void IGC::WritePoint(const Point& point) {
	WritePoint(point.GetCenterPoint());
}

void IGC::WriteCircle(const Circle& circle) {
	WritePoint(circle.GetCenterPoint(),true);
	file << "DC " << circle.GetRadiusNM() << "\r\n";
}

void IGC::WriteSector(const Sector& sector) {
	if (varRotationClockwise != sector.IsClockwise()) { // Write var if changed
		varRotationClockwise = !varRotationClockwise;
		file << "V D=" << (varRotationClockwise ? "+" : "-") << "\r\n";
	}
	WritePoint(sector.GetCenterPoint(),true);
	int dir1, dir2;
	if (Geometry::IsInt(sector.GetAngleStart(),dir1) && Geometry::IsInt(sector.GetAngleEnd(),dir2))
		file << "DA " << sector.GetRadiusNM() << "," << dir1 << "," << dir2;
	else {
		file << "DB ";
		WritePoint(sector.GetStartPoint(),true,false);
		file << ",";
		WritePoint(sector.GetEndPoint(),true,false);
	}
	file << "\r\n";
}
#endif
