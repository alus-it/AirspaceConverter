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

#include "OpenAir.h"
#include "AirspaceConverter.h"
#include "Airspace.h"
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

std::string& OpenAir::RemoveComments(std::string &s) {
	s.erase(find_if(s.begin(), s.end(), [](const char c) { return c == '*'; }), s.end());
	return s;
}

bool OpenAir::ParseDegrees(const std::string& dddmmss, double& deg) {
	if(dddmmss.empty()) return false;

	// Tokenize on colons
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

bool OpenAir::ParseCoordinates(const std::string& text, Geometry::LatLon& point) {
	// Tokenize on spaces
	boost::tokenizer<boost::char_separator<char> > tokens(text, boost::char_separator<char>(" "));
	if(std::distance(tokens.begin(),tokens.end()) < 2) return false; // We expect at least 2 fields

	// Latitude degrees, minutes and seconds
	boost::tokenizer<boost::char_separator<char>>::iterator token=tokens.begin();
	std::string coord(*token);
	char sign;
	if(coord.size()>1 && !isDigit(coord.back())) {
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

	// Longitude degrees, minutes and seconds
	if (++token == tokens.end()) return false;
	coord = *token;
	if(coord.size()>1 && !isDigit(coord.back())) {
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

	// Finally set the poit coordinates
	point.SetLatLon(lat,lon);
	return true;
}

OpenAir::OpenAir(std::multimap<int, Airspace>& airspacesMap)
	: airspaces(&airspacesMap)
	, varRotationClockwise(true)
	/*, varWidth(0) */ {
}

// Reading and parsing OpenAir airspace file
bool OpenAir::ReadFile(const std::string& fileName) {
	assert(airspaces != nullptr);
	if (airspaces == nullptr) return false;
	std::ifstream input(fileName, std::ios::binary);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open input file: " + fileName, true);
		return false;
	}
	AirspaceConverter::LogMessage("Reading OpenAir file: " + fileName, false);
	int linecount = 0;
	std::string sLine;
	bool allParsedOK = true, isCRLF = false, CRLFwarningGiven = false;
	Airspace airspace;
	while (!input.eof() && input.good()) {

		// Get the line
		AirspaceConverter::SafeGetline(input, sLine, isCRLF);
		++linecount;

		// Verify line ending
		if (!CRLFwarningGiven && !isCRLF) {
			AirspaceConverter::LogMessage(boost::str(boost::format("WARNING on line %1d: not valid Windows style end of line (expected CR LF).") % linecount), true);
			AirspaceConverter::LogMessage("This warning will be not reapeated for furter lines not termintated with CR LF of this OpenAir file.", false);

			// OpenAir files may contain thousends of lines we don't want to print this warning all the time
			CRLFwarningGiven = true;
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

		// Skip too short lines
		if (sLine.size() <= 2) continue;

		bool lineParsedOK = true;
		switch (sLine.at(0)) {
		case 'A':
			switch(sLine.at(1)) {
			case 'C': //AC
				lineParsedOK = ParseAC(sLine, airspace);
				break;
			case 'N': //AN
				lineParsedOK = ParseAN(sLine, airspace);
				break;
			case 'L': //AL
				lineParsedOK = ParseAltitude(sLine, false, airspace);
				break;
			case 'H': //AH
				lineParsedOK = ParseAltitude(sLine, true, airspace);
				break;
			case 'T': //AT
			case 'F': //AF
			case 'G': //AG
			case 'Y': //AY // ignore all those
				break;
			default:
				lineParsedOK = false;
				break;
			} // A
			break;
		case 'D':
			switch (sLine.at(1)) {
			case 'P': // DP
				lineParsedOK = ParseDP(sLine, airspace);
				break;
			case 'A': // DA
				lineParsedOK = ParseDA(sLine, airspace);
				break;
			case 'B': // DB
				lineParsedOK = ParseDB(sLine, airspace);
				break;
			case 'C': // DC
				lineParsedOK = ParseDC(sLine, airspace);
				break;
			case 'Y': // DY
				//ParseDY(sLine, airspace); // Airway not yet supported
				AirspaceConverter::LogMessage(boost::str(boost::format("Warning: skipping airway segment (not yet supported) on line %1d: %2s") %linecount %sLine), false);
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
			continue;
		}
		if (!lineParsedOK) {
			AirspaceConverter::LogMessage(boost::str(boost::format("ERROR: unable to parse OpenAir line %1d: %2s") %linecount %sLine), true);
			allParsedOK = false;
		}
	}
	InsertAirspace(airspace);
	input.close();
	return allParsedOK;
}

bool OpenAir::ParseAC(const std::string & line, Airspace& airspace) {
	varRotationClockwise = true; // Reset var to default at beginning of new airspace segment
	InsertAirspace(airspace); // If new airspace first store the actual one
	assert(airspace.GetType() == Airspace::UNDEFINED);
	Airspace::Type type = Airspace::UNDEFINED;
	size_t length(line.size());
	if (length == 4) switch (line.at(3)) {
		case 'R': type = Airspace::RESTRICTED; break; // restricted
		case 'Q': type = Airspace::DANGER; break; // danger
		case 'P': type = Airspace::PROHIBITED; break; // prohibited
		case 'A': type = Airspace::CLASSA; break; // Class A
		case 'B': type = Airspace::CLASSB; break; // Class B
		case 'C': type = Airspace::CLASSC; break; // Class C
		case 'D': type = Airspace::CLASSD; break; // Class D
		case 'E': type = Airspace::CLASSE; break; // Class E
		case 'F': type = Airspace::CLASSF; break; // Class F
		case 'G': type = Airspace::CLASSG; break; // Class G
		case 'W': type = Airspace::WAVE; break; // Wave Window
	} else if (length == 6) {
		if (line.at(3) == 'C' && line.at(4) == 'T' && line.at(5) == 'R') type = Airspace::CTR;
		else if (line.at(4) == 'M' && line.at(5) == 'Z') {
			if (line.at(3) == 'T') type = Airspace::TMZ;
			else if (line.at(3) == 'R') type = Airspace::RMZ;
		}
	} else if (length == 5 && line.at(3)=='G' && line.at(4) == 'P') type = Airspace::NOGLIDER; //GP glider prohibited
	else if (length == 10 && line.substr(3) == "UNKNOWN") type = Airspace::UNKNOWN; // UKNOWN can be used in OpneAir
	if (type == Airspace::UNDEFINED) return false;
	airspace.SetType(type);
	return true;
}

bool OpenAir::ParseAN(const std::string & line, Airspace& airspace) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	if (line.size() < 4) return false;
	airspace.SetName(line.substr(3));
	return true;
}

bool OpenAir::ParseAltitude(const std::string& line, const bool isTop, Airspace& airspace) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	const int l = (int)line.length();
	if (l < 4) return false;
	double value = 0;
	bool isFL = false;
	bool isAGL = false;
	bool isAMSL = true;
	bool valueFound = false;
	bool typeFound = false;
	bool isMeter = false;
	bool unitFound = false;
	int s = 3;
	bool isNumber = isDigit(line.at(s));
	for (int i = 4; i < l; i++) {
		const char c = line.at(i);
		const bool isLast = (i == l - 1);
		const bool isSep = (c == ' ' || c == '=');
		if (isDigit(c) != isNumber || isSep || isLast ) {
			const std::string str = isLast ? line.substr(s) : line.substr(s, i - s);
			if (isNumber) {
				if (!valueFound) {
					try {
						value = std::stod(str);
					} catch (...) {
						return false;
					}
					valueFound = true;
				}
				else return false;
			} else {
				if (!typeFound) {
					if (valueFound) {
						if (boost::iequals(str,"AGL") || boost::iequals(str,"AGND") || boost::iequals(str,"ASFC") || boost::iequals(str,"GND") || boost::iequals(str,"SFC")) {
							isAGL = true;
							isAMSL = false;
							typeFound = true;
						} else if (boost::iequals(str, "MSL") || boost::iequals(str, "AMSL") || boost::iequals(str, "ALT")) typeFound = true;
						else if (!unitFound) {
							if (boost::iequals(str, "FT") || boost::iequals(str, "F")) unitFound = true;
							else if (boost::iequals(str, "M") || boost::iequals(str, "MT")) {
								isMeter = true;
								unitFound = true;
							}
						} 	
					} else {
						if (boost::iequals(str, "FL")) {
							isFL = true;
							isAMSL = false;
							typeFound = true;
						} else if (boost::iequals(str, "GND") || boost::iequals(str, "SFC")) {
							isAGL = true;
							isAMSL = false;
							typeFound = true;
							valueFound = true;
							unitFound = true;
						} else if (boost::iequals(str, "UNLIM") || boost::iequals(str, "UNLIMITED")) {
							isAGL = false;
							isAMSL = true;
							typeFound = true;
							valueFound = true;
							unitFound = true;
							value = 10000000;
						}
					}
				} else if (!unitFound && !typeFound) return false;
			}
			if (valueFound && typeFound && unitFound) break;
			if (line.at(i) == ' ' || line.at(i) == '=') {
				i++;
				if (i < l) isNumber = isDigit(line.at(i));
			}
			else isNumber = !isNumber;
			s = i;
		}
	}
	Altitude alt;
	if (isFL) alt.SetFlightLevel((int)value);
	else if (isAMSL) isMeter ? alt.SetAltMtMSL(value) : alt.SetAltFtMSL((int)value);
	else if (isAGL) isMeter ? alt.SetAltMtGND(value) : alt.SetAltFtGND((int)value);
	isTop ? airspace.SetTopAltitude(alt) : airspace.SetBaseAltitude(alt);
	return true;
}

bool OpenAir::ParseS(const std::string & line) {
	if (line.size() > 1 && (line.at(1) == 'P' || line.at(1) == 'B')) return true; // ignore it...
	return false;
}

bool OpenAir::ParseT(const std::string& line) {
	if (line.size() > 1 && (line.at(1) == 'C' || line.at(1) == 'O')) return true; // Style, pen or brush record ignore it...
	return false;
}

bool OpenAir::ParseDP(const std::string& line, Airspace& airspace) {
	if (airspace.GetType() == Airspace::UNDEFINED) return true;
	if (line.length() < 14) return false;
	Geometry::LatLon point;
	if (ParseCoordinates(line.substr(3), point)) {
		airspace.AddPoint(point);
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

bool OpenAir::ParseDA(const std::string& line, Airspace& airspace) {
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
	assert(airspaces != nullptr);
	if (airspaces == nullptr) return false;
	const bool validAirspace = airspace.GetType() != Airspace::UNDEFINED && !airspace.GetName().empty() && airspace.GetNumberOfGeometries() > 0 && !airspace.GetTopAltitude().IsGND();
	if (validAirspace) {
		airspace.ClosePoints();
		assert(airspace.GetNumberOfPoints() > 3);
		airspaces->insert(std::pair<int, Airspace>(airspace.GetType(),std::move(airspace)));
	} else airspace.Clear();
	return validAirspace;
}

bool OpenAir::WriteFile(const std::string& fileName) {
	if (airspaces == nullptr) return false;
	if (file.is_open()) file.close();
	file.open(fileName, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file.is_open() || file.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open output file: " + fileName, true);
		return false;
	}
	AirspaceConverter::LogMessage("Writing output file: " + fileName, false);

	WriteHeader();

	// Go trough all airspace
	for (std::pair<const int,Airspace>& pair : *airspaces)
	{
		// Get the airspace
		Airspace& a = pair.second;

		// Just a couple if assertions
		assert(a.GetNumberOfPoints() > 3);
		assert(a.GetFirstPoint()==a.GetLastPoint());

		// Reset var
		varRotationClockwise = true;

		// Skip OpenAir not supported categories
		if (!WriteCategory(a)) continue;
		
		// Write the name
		file << "AN " << a.GetName() << "\r\n";
		
		// Write base and ceiling altitudes
		file << "AL " << a.GetBaseAltitude().ToString() << "\r\n";
		file << "AH " << a.GetTopAltitude().ToString() << "\r\n";

		// Get number of geometries
		unsigned int numOfGeometries = a.GetNumberOfGeometries();

		// If no geometries are defined we have to calculate them
		if (numOfGeometries == 0) {
			a.Undiscretize();
			numOfGeometries = a.GetNumberOfGeometries();
		}
		assert(numOfGeometries > 0);

		// Write each geometry
		for (unsigned int i = 0; i < numOfGeometries; i++) a.GetGeometryAt(i)->WriteOpenAirGeometry(this);

		// Add an empty line at the end of the airspace
		file << "\r\n";
	}
	file.close();
	return true;
}

void OpenAir::WriteHeader() {
	for(const std::string& line: AirspaceConverter::disclaimer) file << "* " << line << "\r\n";
	file << "\r\n";
}

bool OpenAir::WriteCategory(const Airspace& airspace) {
	std::string openAirCategory;
	switch(airspace.GetType()) {
		case Airspace::RESTRICTED:	openAirCategory = "R"; break;
		case Airspace::DANGER:		openAirCategory = "Q"; break;
		case Airspace::PROHIBITED:	openAirCategory = "P"; break;
		case Airspace::CTR:			openAirCategory = "CTR"; break;
		case Airspace::CLASSA:		openAirCategory = "A"; break;
		case Airspace::CLASSB:		openAirCategory = "B"; break;
		case Airspace::CLASSC:		openAirCategory = "C"; break;
		case Airspace::CLASSD:		openAirCategory = "D"; break;
		case Airspace::CLASSE:		openAirCategory = "E"; break;
		case Airspace::CLASSF:		openAirCategory = "F"; break;
		case Airspace::CLASSG:		openAirCategory = "G"; break;
		case Airspace::WAVE:		openAirCategory = "W"; break;
		case Airspace::RMZ:			openAirCategory = "RMZ"; break;
		case Airspace::TMZ:			openAirCategory = "TMZ"; break;
		case Airspace::NOGLIDER:	openAirCategory = "GP"; break;
		case Airspace::UNKNOWN:		openAirCategory = "UNKNOWN"; break;
		default: return false; //cases not existent in OpenAir: TMA, FIR, UIR, OTH, GLIDING, UNDEFINED
	}
	file << "AC " << openAirCategory << "\r\n";
	return true;
}

void OpenAir::WriteLatLon(const Geometry::LatLon& point) {
	int deg;
	double min;
	point.GetLatDegMin(deg, min);
	file << deg << ":" << min << " " << point.GetNorS() << " ";
	point.GetLonDegMin(deg, min);
	file << deg << ":" << min << " " << point.GetEorW();
}

void OpenAir::WritePoint(const Point* point) {
	assert(point != nullptr);
	if (point == nullptr) return;
	file << "DP ";
	WriteLatLon(point->GetCenterPoint());
	file << "\r\n";
}

void OpenAir::WriteCircle(const Circle* circle) {
	assert(circle != nullptr);
	if (circle == nullptr) return;
	file << "V X=";
	WriteLatLon(circle->GetCenterPoint());
	file << "\r\nDC " << circle->GetRadiusNM() << "\r\n";
}

void OpenAir::WriteSector(const Sector* sector) {
	assert(sector != nullptr);
	if (sector == nullptr) return;
	if (varRotationClockwise != sector->IsClockwise()) { // Write var if changed
		varRotationClockwise = !varRotationClockwise;
		file << "V D=" << (varRotationClockwise ? "+" : "-") << "\r\n";
	}
	file << "V X=";
	WriteLatLon(sector->GetCenterPoint());
	file << "\r\nDB ";
	WriteLatLon(sector->GetStartPoint());
	file << ",";
	WriteLatLon(sector->GetEndPoint());
	file << "\r\n";
}
