//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
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

std::string& OpenAir::RemoveComments(std::string &s) {
	s.erase(find_if(s.begin(), s.end(), [](const char c) { return c == '*'; }), s.end());
	return s;
}

bool OpenAir::ParseDegrees(const std::string& dddmmss, double& deg) {
	boost::char_separator<char> sep(":");
	boost::tokenizer<boost::char_separator<char>> tokens(dddmmss, sep);
	deg = 0;
	int i = 0;
	for (const std::string& c : tokens) {
		assert(!c.empty());
		if (!isDigit(c.front())) return false;
		switch (i) {
		case 0:
			deg += std::stoi(c);
			break;
		case 1:
			deg += std::stod(c) / 60;
			break;
		case 2:
			deg += std::stod(c) / 3600;
			break;
		default:
			return false;
		}
		i++;
	}
	return true;
}

bool OpenAir::ParseCoordinates(const std::string& text, Geometry::LatLon& point)
{
	boost::char_separator<char> sep(" ");
	boost::tokenizer<boost::char_separator<char> > tokens(text, sep);
	int i = 0;
	double lat = 0, lon = 0;
	for (const std::string& c : tokens)
	{
		if (c.empty()) return false;
		switch (i) {
		case 0:
			if (!ParseDegrees(c, lat)) return false;
			break;
		case 1:
			if (c.length() == 1) {
				const char NS = c.front();
				if (NS == 'S' || NS == 's') lat = -lat;
				else if (NS != 'N' && NS != 'n') return false;
			} else return false;
			break;
		case 2:
			if (!ParseDegrees(c, lon)) return false;
			break;
		case 3:
			if (c.length() == 1) {
				const char EW = c.front();
				if (EW == 'W' || EW == 'w') lat = -lat;
				else if (EW != 'E' && EW != 'e') return false;
			} else return false;
			break;
		default:
			return false;
		}
		i++;
	}
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
	std::ifstream input(fileName);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open input file: " + fileName, true);
		return false;
	}
	AirspaceConverter::LogMessage("Reading OpenAir file: " + fileName, false);
	int linecount = 0;
	bool allParsedOK = true;
	Airspace airspace;
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
				AirspaceConverter::LogMessage("Warning: skipping airway segment which is not yet supported: " + sLine, false);
				assert(false); // Airway not yet supported
				lineParsedOK = false; //ParseDY(sLine, airspace);
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
			AirspaceConverter::LogMessage("ERROR: unable to parse line: " + sLine, true);
			assert(false);
			allParsedOK = false;
		}
	}
	InsertAirspace(airspace);
	input.close();
	return allParsedOK;
}

bool OpenAir::ParseAC(const std::string & line, Airspace& airspace)
{
	InsertAirspace(airspace); // If new airspace first store the actual one
	assert(airspace.GetType() == Airspace::UNKNOWN);
	Airspace::Type type = Airspace::UNKNOWN;
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
		else if (line.at(3) == 'T' && line.at(4) == 'M' && line.at(5) == 'Z') type = Airspace::TMZ;
	} else if (length == 5 && line.at(3)=='G' && line.at(4) == 'P') type = Airspace::NOGLIDER; //GP glider prohibited
	if (type == Airspace::UNKNOWN) return false;
	airspace.SetType(type);
	return true;
}

bool OpenAir::ParseAN(const std::string & line, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNKNOWN) return true;
	if (line.size() < 4) return false;
	airspace.SetName(line.substr(3));
	return true;
}

bool OpenAir::ParseAltitude(const std::string& line, const bool isTop, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNKNOWN) return true;
	const int l = line.length();
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
					value = std::stod(str);
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

bool OpenAir::ParseS(const std::string & line)
{
	if (line.size() > 1 && (line.at(1) == 'P' || line.at(1) == 'B')) return true; // ignore it...
	return false;
}

bool OpenAir::ParseT(const std::string& line)
{
	if (line.size() > 1 && (line.at(1) == 'C' || line.at(1) == 'O')) return true; // Style, pen or brush record ignore it...
	return false;
}

bool OpenAir::ParseDP(const std::string& line, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNKNOWN) return true;
	if (line.length() < 14) return false;
	Geometry::LatLon point;
	if (ParseCoordinates(line.substr(3), point)) {
		airspace.AddPoint(point);
		return true;
	}
	return false;
}

bool OpenAir::ParseV(const std::string & line, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNKNOWN) return true;
	if (line.length() < 5) return false;
	switch (line.at(2)) {
	case 'D':
		{
			const char c = line.at(4);
			if (c == '+') varRotationClockwise = true;
			else if(c == '-') varRotationClockwise = false;
			else {
				varRotationClockwise = true;
				return false;
			}
		}
		break;
	case 'X':
		{
			if (ParseCoordinates(line.substr(4), varPoint)) return true;
			varPoint.SetLatLon(Geometry::LatLon::UNDEF_LAT, Geometry::LatLon::UNDEF_LON);
		}
		return false;
	case 'W':
		/*if (isDigit(line.at(4))) varWidth = std::stod(line.substr(4));
		else {
			varWidth = 0;
			return false;
		}*/
		break;
	case 'Z': // ingnore it
		break;
	default:
		return false;
	}
	return true;
}

bool OpenAir::ParseDA(const std::string& line, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNKNOWN) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 8) return false;
	boost::char_separator<char> sep(",");
	std::string data(line.substr(3));
	boost::tokenizer<boost::char_separator<char>> tokens(data, sep);
	double radius, angleStart, angleEnd;
	int i = 0;
	for (const std::string& c : tokens) {
		switch (i) {
		case 0:
			radius = std::stod(c);
			break;
		case 1:
			angleStart = std::stod(c);
			break;
		case 2:
			angleEnd = std::stod(c);
			break;
		default:
			break;
		}
		i++;
	}
	airspace.AddGeometry(new Sector(varPoint, radius, angleStart, angleEnd, varRotationClockwise));
	return true;
}

bool OpenAir::ParseDB(const std::string& line, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNKNOWN) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 26) return false;
	boost::char_separator<char> sep(",");
	std::string data(line.substr(3));
	boost::tokenizer<boost::char_separator<char>> tokens(data, sep);
	Geometry::LatLon p1, p2;
	int i = 0;
	for (const std::string& c : tokens) {
		switch (i) {
		case 0:
			if (!ParseCoordinates(c, p1)) return false;
			break;
		case 1:
			if (!ParseCoordinates(c, p2)) return false;
			break;
		default:
			break;
		}
		i++;
	}
	airspace.AddGeometry(new Sector(varPoint, p1, p2, varRotationClockwise));
	return true;
}

bool OpenAir::ParseDC(const std::string& line, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNKNOWN) return true;
	if (varPoint.Lat() == Geometry::LatLon::UNDEF_LAT || line.length() < 4 || !isDigit(line.at(3))) return false;
	double radius = std::stod(line.substr(3));
	airspace.AddGeometry(new Circle(varPoint, radius));
	return true;
}

/* Airway not yet supported
bool OpenAir::ParseDY(const std::string & line, Airspace& airspace)
{
	if (airspace.GetType() == Airspace::UNKNOWN) return true;
	if (varWidth == 0 || line.length() < 14) return false;
	double lat = 0, lon = 0;
	if (ParseCoordinates(line.substr(3), lat, lon)) {
		airspace.AddGeometry(new AirwayPoint(lat, lon, varWidth));
		return true;
	} 
	return false;
}
*/

void OpenAir::ResetVar()
{
	varRotationClockwise = true;
	varPoint.SetLatLon(Geometry::LatLon::UNDEF_LAT,Geometry::LatLon::UNDEF_LON);
	//varWidth = 0;
}

bool OpenAir::InsertAirspace(Airspace& airspace)
{
	if (airspaces == nullptr) {
		assert(false);
		return false;
	}
	const bool validAirspace = airspace.GetType() != Airspace::UNKNOWN && !airspace.GetName().empty() && airspace.GetNumberOfGeometries() > 0 && !airspace.GetTopAltitude().IsGND();
	if (validAirspace) {
		assert(airspace.GetNumberOfPoints() > 0);
		airspace.ClosePoints();
		airspaces->insert(std::pair<int, Airspace>(airspace.GetType(),std::move(airspace)));
	} else airspace.Clear();
	ResetVar();
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

		// Skip OpenAir not supported categories
		if (!WriteCategory(a)) continue;
		
		// Write the name
		file << "AN " << a.GetName() << "\r\n";
		
		// Write base and ceiling altitudes
		file << "AL " << a.GetBaseAltitude().ToString() << "\r\n";
		file << "AH " << a.GetTopAltitude().ToString() << "\r\n";

		// If no geometries are defined we have to calculate them
		if (a.GetNumberOfGeometries() == 0) a.Undiscretize();
		
		const unsigned int numOfGeometries = a.GetNumberOfGeometries();
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
	for(const std::string& line: AirspaceConverter::disclaimer) file << "*" << line << "\r\n";
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
		case Airspace::TMZ:			openAirCategory = "TMZ"; break;
		case Airspace::NOGLIDER:	openAirCategory = "GP"; break;
		default: return false; //cases not existent in OpenAir: TMA, RMZ, FIR, UIR, OTH, GLIDING, UNKNOWN
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

void OpenAir::WritePoint(const Point* point)
{
	assert(point != nullptr);
	if (point == nullptr) return;
	file << "DP ";
	WriteLatLon(point->GetCenterPoint());
	file << "\r\n";
}

void OpenAir::WriteCircle(const Circle* circle)
{
	assert(circle != nullptr);
	if (circle == nullptr) return;
	file << "V X=";
	WriteLatLon(circle->GetCenterPoint());
	file << "\r\nDC " << circle->GetRadiusNM() << "\r\n";
}

void OpenAir::WriteSector(const Sector* sector)
{
	assert(sector != nullptr);
	if (sector == nullptr) return;
	if (!sector->IsClockwise()) file << "X D=-\r\n";
	file << "V X=";
	WriteLatLon(sector->GetCenterPoint());
	file << "\r\nDB ";
	WriteLatLon(sector->GetStartPoint());
	file << ",";
	WriteLatLon(sector->GetEndPoint());
	file << "\r\n";
}
