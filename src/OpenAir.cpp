//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "OpenAir.hpp"
#include "AirspaceConverter.hpp"
#include <iomanip>
#include <format>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/locale/encoding.hpp>

const std::unordered_map<std::string, Airspace::Type> OpenAir::openAirAirspaceTable = {
	{ "A", Airspace::CLASSA },
	{ "B", Airspace::CLASSB },
	{ "C", Airspace::CLASSC },
	{ "D", Airspace::CLASSD },
	{ "E", Airspace::CLASSE },
	{ "F", Airspace::CLASSF },
	{ "G", Airspace::CLASSG },
	{ "Q", Airspace::D },
	{ "P", Airspace::P },
	{ "R", Airspace::R },
	{ "CTR", Airspace::CTR },
	{ "TMZ", Airspace::TMZ },
	{ "RMZ", Airspace::RMZ },
	{ "GSEC", Airspace::GLIDING },
	{ "GP", Airspace::NOGLIDER },
	{ "W", Airspace::WAVE },
	{ "WAVE", Airspace::WAVE },
	{ "NOTAM", Airspace::NOTAM },
	{ "OTHER", Airspace::OTHER },
	{ "TMA", Airspace::TMA },
	{ "FIR", Airspace::FIR },
	{ "UIR", Airspace::UIR },
	{ "OTH", Airspace::OTH },
	{ "AWY", Airspace::AWY },
	{ "MATZ", Airspace::MATZ },
	{ "MTMA", Airspace::MTMA },
	{ "MTRA", Airspace::MTRA },
	{ "T", Airspace::TFR },
	{ "TFR", Airspace::TFR },
	{ "ADA", Airspace::ADA },
	{ "ADIZ", Airspace::ADIZ },
	{ "CTA", Airspace::CTR },
	{ "DFIR", Airspace::DFIR },
	{ "TIZ", Airspace::TIZ },
	{ "TIA", Airspace::TIA },
	{ "SRZ", Airspace::SRZ },
	{ "ATZ", Airspace::ATZ },
	{ "FISA", Airspace::FISA },
	{ "MBZ", Airspace::MBZ },
	{ "ASR", Airspace::ASR },
	{ "COMP", Airspace::COMP },
	{ "TRZ", Airspace::TRZ },
	{ "VFRR", Airspace::VFRR },
	{ "RTZ", Airspace::RTZ },
	{ "PARA", Airspace::PARA },
	{ "LFZ", Airspace::LFZ },
	{ "CFZ", Airspace::CFZ },
	{ "MOA", Airspace::MOA },
	{ "MTA", Airspace::MTA },
	{ "TSA", Airspace::TSA },
	{ "TRA", Airspace::TRA },
	{ "UKN", Airspace::UNKNOWN },
	{ "UNKNOWN", Airspace::UNKNOWN }
};

bool OpenAir::calculateArcs = true;
bool OpenAir::lastPointWasEqualToFirst = false;
OpenAir::CoordinateType OpenAir::coordinateType = OpenAir::CoordinateType::AUTO;
int OpenAir::linecount = 0;

OpenAir::OpenAir(std::multimap<int, Airspace>& airspacesMap):
	airspaces(airspacesMap),
	varRotationClockwise(true),
	lastACline(-1),
	lastPointWasDDMMSS(false),
	lastLatD(Geometry::LatLon::UNDEF_LAT),
	lastLatM(0),
	lastLatS(0),
	lastLonD(Geometry::LatLon::UNDEF_LON),
	lastLonM(0),
	lastLonS(0) {
}

std::string& OpenAir::RemoveComments(std::string &s) {
	s.erase(find_if(s.begin(), s.end(), [](const char c) { return c == '*'; }), s.end());
	return s;
}

bool OpenAir::ParseDegrees(const std::string& dddmmss, double& deg, bool isLon) {
	// The OpenAir coordinate string can't be empty
	if(dddmmss.empty()) return false;

	// The OpenAir coordinate string must contain only numbers, points and colons ':'
	if(std::find_if(dddmmss.begin(), dddmmss.end(), [](char c) { return !std::isdigit(c) && c != ':' && c != '.'; }) != dddmmss.end()) return false;

	// Tokenize on columns
	boost::tokenizer<boost::char_separator<char>> tokens(dddmmss, boost::char_separator<char>(":"));
	const int fields = (int)std::distance(tokens.begin(),tokens.end());
	if(fields < 2 || fields > 3) return false; // We expect from 2 to 3 fields
	boost::tokenizer<boost::char_separator<char>>::iterator token=tokens.begin();
	if ((*token).empty()) return false;
	try {
		bool warnDegDigits = false, warnMinDigits = false;

		// Degrees
		if ((*token).length() != (isLon ? 3 : 2)) warnDegDigits = true;
		deg = std::stod(*token);
		if (deg < 0 || deg >= (isLon ? 180 : 90)) {
			AirspaceConverter::LogError(std::format("on line {}: invalid value of degrees for {}.", linecount, isLon ? "longitude" : "latitude"));
			return false;
		}

		// Minutes
		if (++token != tokens.end()) {
			if ((*token).empty()) return false;
			if (fields == 3 && (*token).length() != 2) warnMinDigits = true;
			const double min = std::stod(*token);
			if (min < 0 || min >= 60) {
				AirspaceConverter::LogError(std::format("on line {}: invalid value of minutes for {}.", linecount, isLon ? "longitude" : "latitude"));
				return false;
			}
			deg += min / 60;

			// Seconds
			if (++token != tokens.end()) {
				if ((*token).empty()) return false;
				if (warnDegDigits)
					AirspaceConverter::LogWarning(std::format("on line {}: wrong number of digits for {} degrees.", linecount, isLon ? "longitude" : "latitude"));
				if (warnMinDigits)
					AirspaceConverter::LogWarning(std::format("on line {}: wrong number of digits for {} minutes.", linecount, isLon ? "longitude" : "latitude"));
				if ((*token).length() != 2)
					AirspaceConverter::LogWarning(std::format("on line {}: wrong number of digits for {} seconds.", linecount, isLon ? "longitude" : "latitude"));
				const double sec = std::stod(*token);
				if (sec < 0 || sec >= 60) {
					AirspaceConverter::LogError(std::format("on line {}: invalid value of seconds for {}.", linecount, isLon ? "longitude" : "latitude"));
					return false;
				}
				deg += sec / 3600;
			}
		}
	} catch (...) {
		return false;
	}

	// Check if the final value is valid
	if (deg > (isLon ? 180 : 90)) {
		AirspaceConverter::LogError(std::format("on line {}: invalid {} value.", linecount, isLon ? "longitude" : "latitude"));
		return false;
	}

	return true;
}

bool OpenAir::ParseCoordinates(const std::string& text, Geometry::LatLon& point) {
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
	if (!ParseDegrees(coord, lat, false)) return false;

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
	if (!ParseDegrees(coord, lon, true)) return false;

	// Apply the longitude sign E or W
	if (sign == 'W' || sign == 'w') lon = -lon;
	else if (sign != 'E' && sign != 'e') return false;

	// Verify validity of longitude
	if (!Geometry::LatLon::IsValidLon(lon)) return false;

	// Finally set the point coordinates
	point.SetLatLon(lat,lon);
	return true;
}

bool OpenAir::IsFileUTF8(std::ifstream& inputFile) {
	if (inputFile.get() == 0xef && inputFile.get() == 0xbb && inputFile.get() == 0xbf) return true; // Check if first three characters are the UTF-8 BOM
	inputFile.seekg(0); //re-wind
	//TODO: Very few UTF-8 file has BOM, so here we should scan the file and verify if it is UTF-8
	return false;
}

// Reading and parsing OpenAir airspace file
bool OpenAir::Read(const std::string& fileName) {
	std::ifstream input(fileName, std::ios::binary);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogError("Unable to open input file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Reading OpenAir file: " + fileName);

	// Check if the input file is encoded in UTF-8
	const bool isUTF8 = IsFileUTF8(input);

	linecount = 0;
	std::string sLine;
	bool allParsedOK = true, needToDetectCRLF = true, initialCRLF = false, isCRLF = false, lineEndingConsistent = true;
	Airspace airspace;
	while (!input.eof() && input.good()) {

		// Get the line
		AirspaceConverter::SafeGetline(input, sLine, isCRLF);
		++linecount;

		// Verify line ending at first line
		if (needToDetectCRLF) {
			initialCRLF = isCRLF;
			needToDetectCRLF = false;
		}

		// Verify line ending
		if (lineEndingConsistent && isCRLF != initialCRLF && !sLine.empty()) {
			AirspaceConverter::LogWarning(std::format("on line {}: not consistent line ending style, file started with: {}.", linecount, initialCRLF ? "CR LF" : "LF"));

			// OpenAir files may contain thousands of lines we don't want to print this warning all the time
			lineEndingConsistent = false;
		}
		
		// Directly skip empty lines
		if (sLine.empty()) continue;

		// Remove front spaces
		boost::algorithm::trim_left(sLine);

		// If it was a line with only spaces skip it
		if (sLine.empty()) continue;

		// Then directly skip full comment line
		if (sLine.front() == '*') continue;

		// Remove inline comments
		RemoveComments(sLine);

		// Remove back spaces
		boost::algorithm::trim_right(sLine);

		// Check for too short lines
		bool lineParsedOK = sLine.size() > 2;

		if (lineParsedOK) switch (sLine.at(0)) {
		case 'A':
			switch(sLine.at(1)) {
			case 'C': //AC
				lineParsedOK = ParseAC(sLine, airspace);
				if (lineParsedOK) lastACline = linecount;
				break;
			case 'N': //AN
				lineParsedOK = ParseAN(sLine, airspace, isUTF8);
				break;
			case 'L': //AL
				lineParsedOK = ParseAltitude(sLine, false, airspace);
				break;
			case 'H': //AH
				lineParsedOK = ParseAltitude(sLine, true, airspace);
				break;
			case 'F': //AF radio frequency
				lineParsedOK = ParseAF(sLine, airspace, isUTF8);
				break;
			case 'X': //AX: transponder code
				lineParsedOK = airspace.SetTransponderCode(sLine.substr(3));
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
				lineParsedOK = ParseDP(sLine, airspace, linecount);
				break;
			case 'A': // DA
				lineParsedOK = ParseDA(sLine, airspace, linecount);
				break;
			case 'B': // DB
				lineParsedOK = ParseDB(sLine, airspace);
				break;
			case 'C': // DC
				lineParsedOK = ParseDC(sLine, airspace);
				break;
			case 'Y': // DY
				//ParseDY(sLine, airspace); // Airway not yet supported
				AirspaceConverter::LogWarning(std::format("skipping airway segment (not yet supported) on line {}: {}", linecount, sLine));
				lineParsedOK = false; 
				break;
			default:
				lineParsedOK = false;
				break;
			} // D
			break;
		case 'V':
			lineParsedOK = ParseV(sLine, airspace);
			break;
		case 'S':
			lineParsedOK = ParseS(sLine);
			break;
		case 'T':
			lineParsedOK = ParseT(sLine);
			break;
		default:
			lineParsedOK = false;
			break;
		}
		if (!lineParsedOK) {
			AirspaceConverter::LogError(std::format("unable to parse OpenAir line {}: {}", linecount, sLine));
			allParsedOK = false;
		}
	}
	
	// Insert last airspace
	InsertAirspace(airspace);

	input.close();
	return allParsedOK;
}

bool OpenAir::ParseAC(const std::string & line, Airspace& airspace) {
	varRotationClockwise = true; // Reset var to default at beginning of new airspace segment
	InsertAirspace(airspace); // If new airspace first store the actual one
	assert(airspace.GetType() == Airspace::UNDEFINED);
	Airspace::Type type = Airspace::UNDEFINED;
	if (line.size() < 4 || line.at(2) !=' ') return false;
	auto it = openAirAirspaceTable.find(line.substr(3));
	if (it != openAirAirspaceTable.end()) type = it->second;
	if (type == Airspace::UNDEFINED) return false;
	airspace.SetType(type);
	return true;
}

bool OpenAir::ParseAN(const std::string & line, Airspace& airspace, const bool isUTF8) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	if (line.size() < 4) return false;
	if (airspace.GetName().empty()) {
		std::string name(line.substr(3));
		if (name == "COLORENTRY") {
			airspace.SetType(Airspace::UNDEFINED); // Skip Strepla colortable entries
			return true;
		}
		if (airspace.GetType() > Airspace::Type::OTHER && // If the type will be not (yet) recognized by LK8000 (from Type::OTHER on)
			name.rfind(airspace.GetCategoryName()) == std::string::npos) { // ... and the name does not alredy contain it ...
			name.insert (0, airspace.GetCategoryName() + " "); // Then make sure the name contains the type as text
		}
		airspace.SetName(isUTF8 ? name : boost::locale::conv::between(name, "utf-8", "ISO8859-1"));
		return true;
	}
	AirspaceConverter::LogError(std::format("airspace {} has already a name.", airspace.GetName()));
	return false;	
}

bool OpenAir::ParseAF(const std::string& line, Airspace& airspace, const bool isUTF8) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
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
		airspace.AddRadioFrequency(freqHz, isUTF8 ? descr : boost::locale::conv::between(descr, "utf-8", "ISO8859-1"));
		return true;
	} catch(...) {
		return false;
	}
}

bool OpenAir::ParseAltitude(const std::string& line, const bool isTop, Airspace& airspace) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	const std::string::size_type l = line.length();
	if (l < 4) return false;
	return AirspaceConverter::ParseAltitude(line.substr(3,l-3), isTop, airspace);
}

bool OpenAir::ParseS(const std::string & line) {
	if (line.size() > 1 && (line.at(1) == 'P' || line.at(1) == 'B')) return true; // ignore it...
	return false;
}

bool OpenAir::ParseT(const std::string& line) {
	if (line.size() > 1 && (line.at(1) == 'C' || line.at(1) == 'O')) return true; // Style, pen or brush record ignore it...
	return false;
}

bool OpenAir::ParseDP(const std::string& line, Airspace& airspace, const int& linenumber) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	if (line.length() < 14) return false;
	Geometry::LatLon point;
	if (ParseCoordinates(line.substr(3), point)) {
	
		// If adding the point did not succeed because it's a duplicate...
		if (!airspace.AddPoint(point)) {

			// If the last point was not yet detected as equal to the first
			if (!lastPointWasEqualToFirst) {

				// If this point is matching the first point (can happen that last point matches the first point of last geometry) ... 
				size_t numGeo = airspace.GetNumberOfGeometries();
				if (numGeo > 1 && !airspace.GetGeometryAt(numGeo - 1)->IsPoint() && airspace.GetFirstPoint() == point) {
					
					// ... take a note that the last point is matching the fisrt (as it should be)
					lastPointWasEqualToFirst = true;
					return true;
				}
			}

			// If the last point equal to the first was alredy detected than there is really a duplicate
			AirspaceConverter::LogWarning(std::format("skipping unnecessary repeated point on line {}: {}", linenumber, line));	
		}
		return true;
	}
	return false;
}

bool OpenAir::ParseV(const std::string & line, Airspace& airspace) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
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

bool OpenAir::CheckAngleDeg(const double& angleDeg) {
	return angleDeg >= 0 && angleDeg <= 360;
}

bool OpenAir::ParseDA(const std::string& line, Airspace& airspace, const int& linenumber) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 8) return false;
	const std::string data(line.substr(3));
	boost::tokenizer<boost::char_separator<char>> tokens(data, boost::char_separator<char>(","));
	if (std::distance(tokens.begin(), tokens.end()) != 3) return false; // Make sure there are 3 fields
	boost::tokenizer<boost::char_separator<char>>::iterator token = tokens.begin();
	try {
		double radius = std::stod(*token);
		double angleStart = std::stod(*(++token));
		double angleEnd = std::stod(*(++token));
		if (!CheckAngleDeg(angleStart) || !CheckAngleDeg(angleEnd)) AirspaceConverter::LogWarning(std::format("angle not in range 0-360 on line {}: {}", linenumber, line));
		airspace.AddGeometry(new Sector(varPoint, radius, angleStart, angleEnd, varRotationClockwise));
	} catch (...) {
		return false;
	}
	return true;
}

bool OpenAir::ParseDB(const std::string& line, Airspace& airspace) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 26) return false;
	const std::string data(line.substr(3));
	boost::tokenizer<boost::char_separator<char>> tokens(data, boost::char_separator<char>(","));
	if (std::distance(tokens.begin(), tokens.end()) != 2) return false; // Make sure there are 2 fields
	boost::tokenizer<boost::char_separator<char>>::iterator token = tokens.begin();
	Geometry::LatLon p1;
	if (!ParseCoordinates(*token, p1)) return false;
	Geometry::LatLon p2;
	if (!ParseCoordinates(*(++token), p2)) return false;
	airspace.AddGeometry(new Sector(varPoint, p1, p2, varRotationClockwise));
	return true;
}

bool OpenAir::ParseDC(const std::string& line, Airspace& airspace) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 4) return false;
	try {
		airspace.AddGeometry(new Circle(varPoint, std::stod(line.substr(3))));
	} catch (...) {
		return false;
	}
	return true;
}

/* Airway not yet supported
bool OpenAir::ParseDY(const std::string & line, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	if (varWidth == 0 || line.length() < 14) return false;
	double lat = 0, lon = 0;
	if (ParseCoordinates(line.substr(3), lat, lon)) {
		airspace.AddGeometry(new AirwayPoint(lat, lon, varWidth));
		return true;
	} 
	return false;
}
*/

bool OpenAir::InsertAirspace(Airspace& airspace) {
	if (airspace.GetType() == Airspace::UNDEFINED || airspace.GetName().empty()) {
		airspace.Clear();
		lastPointWasEqualToFirst = false;
		return false;
	}

	// Verify if is valid airspace giving a proper explanatory error message
	bool validAirspace(airspace.GetNumberOfGeometries() > 0);

	if (!validAirspace)
		AirspaceConverter::LogError(std::format("at line {}: skip airspace {} with no geometries.", lastACline, airspace.GetName()));

	if (validAirspace && airspace.GetTopAltitude() <= airspace.GetBaseAltitude()) {
		AirspaceConverter::LogError(std::format("at line {}: skip airspace {} with top and base equal or inverted.", lastACline, airspace.GetName()));
		validAirspace = false;
	}

	if (validAirspace && airspace.GetTopAltitude().IsGND()) {
		AirspaceConverter::LogError(std::format("at line {}: skip airspace {} with top at GND.", lastACline, airspace.GetName()));
		validAirspace = false;
	}

	// Remove repeated or very similar consecutive points
	airspace.RemoveTooCloseConsecutivePoints();

	// Ensure that the points are closed
	if (validAirspace && !airspace.ClosePoints()) {
		AirspaceConverter::LogError(std::format("at line {}: skip airspace {} with less than 3 points.", lastACline, airspace.GetName()));
		validAirspace = false;
	}

	// If all OK insert the new airspace
	if (validAirspace) {

		// This should be just a warning
		if (airspace.GetName().empty()) AirspaceConverter::LogWarning(std::format("at line {}: airspace without name.", lastACline));
		
		airspaces.insert(std::pair<int, Airspace>(airspace.GetType(), std::move(airspace)));
	}

	// Otherwise discard it
	else airspace.Clear();

	return validAirspace;	
}

bool OpenAir::Write(const std::string& fileName) {
	if (airspaces.empty()) {
		AirspaceConverter::LogMessage("OpenAir output: no airspace, nothing to write");
		return false;
	}
	if (file.is_open()) file.close();
	file.open(fileName, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file.is_open() || file.bad()) {
		AirspaceConverter::LogError("Unable to open output file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Writing OpenAir output file: " + fileName);

	WriteHeader();

	// Go trough all airspace
	for (std::pair<const int,Airspace>& pair : airspaces)
	{
		// Get the airspace
		Airspace& a = pair.second;

		// Just a couple if assertions
		assert(a.GetNumberOfPoints() > 3);
		assert(a.GetFirstPoint() == a.GetLastPoint());

		// Reset var
		varRotationClockwise = true;

		// Skip OpenAir not supported categories
		if (!WriteCategory(a)) continue;

		// Write the name
		file << "AN " << a.GetName() << "\n";
		
		// Write base and ceiling altitudes
		file << "AL " << a.GetBaseAltitude().ToString() << "\n";
		file << "AH " << a.GetTopAltitude().ToString() << "\n";

		// Write frequencies
		if (a.GetNumberOfRadioFrequencies() > 0) {
			file << std::fixed << std::setprecision(3);
			for (size_t i=0; i<a.GetNumberOfRadioFrequencies(); i++) {
				const std::pair<int, std::string>& f = a.GetRadioFrequencyAt(i);
				file << "AF " << AirspaceConverter::FrequencyMHz(f.first);
				if (!f.second.empty()) file << ' ' << f.second;
				file << "\n";
			}
			file << std::defaultfloat;
		}

		// Write transponder code
		if (a.HasTransponderCode()) file << "AX " << a.GetTransponderCode() << "\n";

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
			for (size_t i = 0; i < numOfGeometries; i++) a.GetGeometryAt(i)->WriteOpenAirGeometry(*this);
		}

		// Otherwise write every single point (except the last one which is the same)
		else for (size_t i = 0; i < a.GetNumberOfPoints() - 1; i++) WritePoint(a.GetPointAt(i));

		// Add an empty line at the end of the airspace
		file << "\n";
	}
	file.close();
	return true;
}

void OpenAir::WriteHeader() {
	file << "*VERSION: 2.0\n";
	file << "*WRITTEN_BY: AirspaceConverter\n";
	file << "*DATE: " << AirspaceConverter::GetCurrentDateString() << "\n";
	for(const std::string& line: AirspaceConverter::disclaimer) file << "* " << line << "\n";
	file << "\n* " << AirspaceConverter::GetFullCreationDateTimeString() << "\n\n";
}

bool OpenAir::WriteCategory(const Airspace& airspace) {
	std::string openAirCategory;
	switch(airspace.GetType()) {
		case Airspace::CLASSA:		openAirCategory = "A"; break;
		case Airspace::CLASSB:		openAirCategory = "B"; break;
		case Airspace::CLASSC:		openAirCategory = "C"; break;
		case Airspace::CLASSD:		openAirCategory = "D"; break;
		case Airspace::CLASSE:		openAirCategory = "E"; break;
		case Airspace::CLASSF:		openAirCategory = "F"; break;
		case Airspace::CLASSG:		openAirCategory = "G"; break;
		case Airspace::D:			openAirCategory = "Q"; break;
		case Airspace::WAVE:		openAirCategory = "W"; break;
		case Airspace::NOGLIDER:	openAirCategory = "GP"; break;
		case Airspace::GLIDING:		openAirCategory = "GSEC"; break;
		case Airspace::UNDEFINED:
			AirspaceConverter::LogWarning(std::format("skipping undefined airspace {}.", airspace.GetName()));
			assert(false);
			return false;
		default: openAirCategory = airspace.CategoryName(airspace.GetType()); break;
	}
	file << "AC " << openAirCategory << "\n";
	lastPointWasDDMMSS = false;
	return true;
}

void OpenAir::WriteLatLonDDMMSS(const int& latD, const int& latM, const int& latS, const char& NorS, const int& lonD, const int& lonM, const int& lonS, const char& EorW) {
	file << std::setw(2) << latD << ":" << std::setw(2) << latM << ":" << std::setw(2) << latS << " " << NorS << " ";
	file << std::setw(3) << lonD << ":" << std::setw(2) << lonM << ":" << std::setw(2) << lonS << " " << EorW;
}

void OpenAir::WriteLatLonDDMMmmm(const int& latD, const double& latM, const char& NorS, const int& lonD, const double& lonM, const char& EorW) {
	file << std::setw(2) << latD << ":" << std::setw(6) << std::fixed << std::setprecision(3) << latM << " " << NorS << " ";
	file << std::setw(3) << lonD << ":" << std::setw(6) << std::setprecision(3) << lonM << " " << EorW;
	file << std::defaultfloat;
}

void OpenAir::WritePoint(const Geometry::LatLon& point, bool isCenterPoint /* = false */, bool addPrefix /*= true*/) {
	if (isCenterPoint && addPrefix) file << "V X=";
	switch (coordinateType) {
		case CoordinateType::DEG_DECIMAL_MIN: {
			if (!isCenterPoint && addPrefix) file << "DP ";
			int latD, lonD;
			double latM, lonM;
			point.GetLatDegMin(latD, latM);
			point.GetLonDegMin(lonD, lonM);
			WriteLatLonDDMMmmm(latD, latM, point.GetNorS(), lonD, lonM, point.GetEorW());
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
			WriteLatLonDDMMSS(latD, latM, latS, point.GetNorS(), lonD, lonM, lonS, point.GetEorW());
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
			if (latDDMMSS && lonDDMMSS) WriteLatLonDDMMSS(latD, latM, latS, point.GetNorS(), lonD, lonM, lonS, point.GetEorW());
			else WriteLatLonDDMMmmm(latD, decimalLatM, point.GetNorS(), lonD, decimalLonM, point.GetEorW());
		}
	}
	if (addPrefix) file << "\n";
}

void OpenAir::WritePoint(const Point& point) {
	WritePoint(point.GetCenterPoint());
}

void OpenAir::WriteCircle(const Circle& circle) {
	WritePoint(circle.GetCenterPoint(), true);
	file << "DC " << circle.GetRadiusNM() << "\n";
}

void OpenAir::WriteSector(const Sector& sector) {
	if (varRotationClockwise != sector.IsClockwise()) { // Write var if changed
		varRotationClockwise = !varRotationClockwise;
		file << "V D=" << (varRotationClockwise ? "+" : "-") << "\n";
	}
	WritePoint(sector.GetCenterPoint(), true);
	int dir1, dir2;
	if (Geometry::IsInt(sector.GetAngleStart(),dir1) && Geometry::IsInt(sector.GetAngleEnd(),dir2))
		file << "DA " << sector.GetRadiusNM() << "," << dir1 << "," << dir2;
	else {
		file << "DB ";
		WritePoint(sector.GetStartPoint(), true, false);
		file << ",";
		WritePoint(sector.GetEndPoint(), true, false);
	}
	file << "\n";
}
