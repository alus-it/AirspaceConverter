//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Authors     : Valerio Messina <efa@iol.it>
//               Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2021 Valerio Messina, Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================
// CSV file format description as used by LittleNavMap, Plan-G, ...
// Are comma separated files, with lines like the following template:
//  Type,Name,Ident,Lat,Lon,Elev,Decl,Label,Desc,Country,Range,ModificationTime,SourceFile
// Note: Type can be: Airport, Airstrip, Bookmark, DME, Helipad, NDB, TACAN, VOR, VORDME, VORTAC, VRP, Waypoint
//       mapping for CUP Style=Type: 2=Airstrip, 4-5=Airport, 9=VOR, 10=NDB, 16=Waypoint/IRP, 17=VRP
// Note: Lat/Lon must be in DDD.MMMMMM format, Elev is in feet and unit must be omitted
// Note: Elevation is always feet and distances are always nautical miles
// Note: LNM fill the Declination field on export, not used when importing
// Note: LNM fill the SourceFile field on export, ignore it when importing and take note internally of source file

#include "CSV.h"
#include "AirspaceConverter.h"
#include "Waypoint.h"
#include "Airfield.h"
#include "Airspace.h"
#include "Geometry.h"
#include <fstream>
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <cmath>
#include <cassert>

CSV::CSV(std::multimap<int,Waypoint*>& waypointsMap):
	waypoints(waypointsMap) {
}

bool CSV::ParseLatitude(const std::string& text, double& lat) {
	const size_t len = text.length();
	if(len < 5) return false;
	try {
		lat = std::stoi(text.substr(0,2));
		lat += std::stod(text.substr(2,len-3))/60;
	} catch(...) {
		return false;
	}
	const char sign = text.back();
	if (sign == 'S' || sign == 's') lat = -lat;
	else if (sign != 'N' && sign != 'n') return false;
	return Geometry::LatLon::IsValidLat(lat);
}

bool CSV::ParseLongitude(const std::string& text, double& lon) {
	const size_t len = text.length();
	if(len < 6) return false;
	try {
		lon = std::stoi(text.substr(0,3));
		lon += std::stod(text.substr(3,len-4))/60;
	} catch (...) {
		return false;
	}
	const char sign = text.back();
	if (sign == 'W' || sign == 'w') lon = -lon;
	else if (sign != 'E' && sign != 'e') return false;
	return Geometry::LatLon::IsValidLon(lon);
}

bool CSV::ParseAltitude(const std::string& text, float& alt) {
	alt = 0;
	size_t pos = text.length() - 1;
	if(pos == 0 && text.front()=='0') return true;
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
	try {
		alt = std::stof(text.substr(0,pos));
		if(feet) alt *= (float)Altitude::FEET2METER;
		return true;
	} catch(...) {}
	return false;
}

bool CSV::ParseStyle(const std::string& text, int& type) {
	if (!text.empty()) try {
		type = std::stoi(text);
		if (type >= Waypoint::unknown && type < Waypoint::numOfWaypointTypes) return true;
	} catch(...) {}
	type = Waypoint::unknown;
	return false;
}

bool CSV::ParseRunwayDir(const std::string& text, int& dir) {
	if (text.empty()) {
		dir = 0; // zero means runway direction unknown
		return true;
	}
	try {
		dir = std::stoi(text);
		if (dir > 0 && dir <= 360) return true;
		if (dir == 0) {
			dir = 360;
			return true;
		}
	} catch(...) {}
	dir = 0;
	return false;
}

bool CSV::ParseRunwayLength(const std::string& text, int& length) {
	length = 0; // zero means invalid/unknown length
	if (text.empty()) return true; // empty field: declared as unknown
	int pos = (int)text.length() - 1;
	if(pos<2) return false; // at least 3 chars: 2 digit and a letter for the unit: a runway of "9m" (2 char) would be not possible...
	bool nauticalMiles = false, statuteMiles = false;
	if(text.back() == 'm' || text.back() == 'M') {
		if(text.at(pos) == 'n' || text.at(pos) == 'N') {
			pos--;
			nauticalMiles = true;
		}
	} else if((text.at(pos-1) == 'm' || text.at(pos-1) == 'M') && (text.back() == 'l' || text.back() == 'L' || text.back() == 'i' || text.back() == 'I')) {
		pos--;
		statuteMiles = true;
	} else return false; // Unable to parse unit
	try {
		double len = std::stod(text.substr(0,pos));
		if (len < 0) return false;
		if (nauticalMiles) len *= Geometry::NM2M;
		else if (statuteMiles) len *= Geometry::MI2M;
		length = (int)std::round(len);
		return true;
	} catch (...) {}
	return false;
}

bool CSV::ParseAirfieldFrequencies(const std::string& text, int& freqHz, int& secondaryFreqHz) {
	freqHz = 0;
	secondaryFreqHz = 0;
	if (text.empty()) return true;
	try {
		size_t pos;
		if (!AirspaceConverter::CheckAirbandFrequency(std::stod(text, &pos),freqHz)) return false;
		if (pos < text.length()) return AirspaceConverter::CheckAirbandFrequency(std::fabs(std::stod(text.substr(pos))),secondaryFreqHz);
		return true;
	} catch(...) {}
	return false;
}

bool CSV::ParseOtherFrequency(const std::string& text, const int type, int& freqHz) {
	freqHz = 0;
	if (text.empty()) return true;
	if (type != Waypoint::WaypointType::VOR && type != Waypoint::WaypointType::NDB) return false; // This waypoint type is not supposed to have a frequency associated
	try {
		const double freq = std::stod(text);
		return type == Waypoint::WaypointType::VOR ? AirspaceConverter::CheckVORfrequency(freq,freqHz) : AirspaceConverter::CheckNDBfrequency(freq,freqHz);
	} catch(...) {}
	return false;
}

#if 0 // as now do not support import of CSV files
bool CSV::Read(const std::string& fileName) {
	std::ifstream input(fileName, std::ios::binary);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogError("Unable to open CUP input file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Reading CUP file: " + fileName);
	int linecount = 0;
	std::string sLine;
	bool isCRLF = false, CRLFwarningGiven = false, firstWaypointFound = false;

	double latitude, longitude;
	int type, runwayDir, runwayLength, radioFreq, altRadioFreq;
	float altitude;

	while (!input.eof() && input.good()) {

		// Get the line
		AirspaceConverter::SafeGetline(input, sLine, isCRLF);
		linecount++;

		// Verify line ending
		if (!CRLFwarningGiven && !isCRLF) {
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: not valid Windows style end of line (expected CR LF).") % linecount));

			// CUP files may contain thousands of WPs we don't want to print this warning all the time
			CRLFwarningGiven = true;
		}

		// Directly skip empty lines
		if (sLine.empty()) continue;

		// Skip eventual header
		if (!firstWaypointFound && (
				sLine.find("name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc") != std::string::npos ||
				sLine.find("name, code, country, lat, lon, elev, style, rwdir, rwlen, freq, desc") != std::string::npos ||
				sLine.find("name, code, country, lat, lon, elev, style, rwydir, rwylen, freq, desc") != std::string::npos)) continue;

		// Remove front spaces
		boost::algorithm::trim_left(sLine);

		// If it was a line with only spaces skip it
		if (sLine.empty()) continue;

		// Then directly skip full comment line
		if (sLine.front() == '*') continue;

		// Remove back spaces
		boost::algorithm::trim_right(sLine);

		// Skip too short lines
		if (sLine.size() <= 10) { // At least ten commas should be there
			AirspaceConverter::LogError(boost::str(boost::format("line %1d is too short to contain anything useful: %2s") %linecount %sLine));
			continue;
		}

		// Check if we arrived to the task section, if yes we're done
		if (sLine == "-----Related Tasks-----") break;

		// Tokenize with quotes
		boost::tokenizer<boost::escaped_list_separator<char> > tokens(sLine); // default separator:',', default quote:'"', default escape char:'\'
		if (std::distance(tokens.begin(),tokens.end()) != 11) { // We expect only 11 fields
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: expected 11 fields: %2s") %linecount %sLine));
			continue;
		}

		// Long name
		boost::tokenizer<boost::escaped_list_separator<char> >::iterator token=tokens.begin();
		const std::string name = *token;
		if (name.empty()) {
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: a name must be present: %2s") %linecount %sLine));
			continue;
		}

		// Code (short name)
		const std::string code = *(++token);

		// Country code
		const std::string country = *(++token);

		// Latitude
		if (!ParseLatitude(*(++token), latitude)) {
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: invalid latitude: %2s") %linecount %(*token)));
			continue;
		}

		// Longitude
		if (!ParseLongitude(*(++token), longitude)) {
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: invalid longitude: %2s") %linecount %(*token)));
			continue;
		}

		// Elevation
		if (!ParseAltitude(*(++token), altitude))
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid elevation: %2s, assuming AMSL") %linecount %(*token)));

		// Waypoint style
		if (!ParseStyle(*(++token),type))
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid waypoint style: %2s, assuming unknown") %linecount %(*token)));

		// If it's an airfield...
		if(Waypoint::IsTypeAirfield((Waypoint::WaypointType)type)) {
			// Runway direction
			if (!ParseRunwayDir(*(++token),runwayDir))
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid runway direction: %2s") % linecount % (*token)));

			// Runway length
			if (!ParseRunwayLength(*(++token),runwayLength))
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid runway length: %2s") %linecount %(*token)));

			// Radio frequency
			if (!ParseAirfieldFrequencies(*(++token),radioFreq,altRadioFreq))
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid radio frequency for airfield: %2s") %linecount %(*token)));

			// Description
			std::string description = *(++token);
			assert(token != tokens.end());

			// Build the airfield
			Airfield* airfield = new Airfield(name, code, country, latitude, longitude, altitude, type, runwayDir, runwayLength, radioFreq, description);
			if (altRadioFreq > 0) {
				assert(radioFreq > 0);
				if (altRadioFreq == radioFreq) AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: skipping repeated secondary radio frequency for airfield.") %linecount));
				else airfield->SetOtherFrequency(altRadioFreq);
			}

			// Add it to the multimap
			waypoints.insert(std::pair<int, Waypoint*>(type, (Waypoint*)airfield));
		} else {
			// Skip runway length and direction
			token++;
			token++;

			// Frequency may be used for VOR and NDB
			if (!ParseOtherFrequency(*(++token), type, radioFreq))
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid frequency for non airfield waypoint: %2s") %linecount %(*token)));

			// Description
			std::string description = *(++token);
			assert(token != tokens.end());

			// Build the waypoint
			Waypoint* waypoint = new Waypoint(name, code, country, latitude, longitude, altitude, type, description);
			if (radioFreq > 0) waypoint->SetOtherFrequency(radioFreq);

			// Add it to the multimap
			waypoints.insert(std::pair<int, Waypoint*>(type, waypoint));
		}

		// Make sure that at this point we already found a valid waypoint so the header is not anymore expected
		if (!firstWaypointFound) firstWaypointFound = true;
	}
	return true;
}
#endif

bool CSV::Write(const std::string& fileName) {
	if (waypoints.empty()) {
		AirspaceConverter::LogMessage("CSV output: no waypoints, nothing to write");
		return false;
	}
	std::ofstream file;
	file.open(fileName, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file.is_open() || file.bad()) {
		AirspaceConverter::LogError("Unable to open output file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Writing CSV output file: " + fileName);

	// Do not write default CSV header on first line, and for compatibilty do not write any other comments
	//file << "#Type,Name,Ident,Lat,Lon,Elev,Decl,Label,Desc,Country,Range,ModificationTime,SourceFile\r\n";

	// Go trough all waypoints
	for (const std::pair<int,Waypoint*>& pair : waypoints) {
		assert(pair.second != nullptr);
		const Waypoint& w(*pair.second);

		// Name is mandatory according to CSV specs
		if (w.GetName().empty()) {
			AirspaceConverter::LogWarning("skipping waypoint with long name empty: " + w.GetCode());
			continue;
		}

		// Waypoint style, 'Type' in CSV spec
		std::string type;
		int style=w.GetType();
		bool fix=1;
		switch(style) {
			case 2:
				type = "Airstrip";
				fix=0;
				break;
			case 4:
			case 5:
				type = "Airport";
				fix=0;
				break;
			case 9:
				type = "VOR";
				break;
			case 10:
				type = "NDB";
				break;
			case 16:
				type = "Waypoint";
				break;
			case 17:
				type = "VRP";
				break;
			default:
				AirspaceConverter::LogWarning(boost::str(boost::format("skipping point with unknown type: %d") % style));
				continue;
		}
		file << type << ',';

		// Long name, 'Name' in CSV spec
		file << w.GetName() << ",";

		// Code "short name", 'Ident' in CSV spec
		if (!w.GetCode().empty()) file << w.GetCode() << ',';

		// Latitude, 'Lat' in CSV spec, expressed as real DD.MMMMMM
		int deg, s=0;
		double min;
		const Geometry::LatLon& pos(w.GetPosition());

		pos.GetLatDegMin(deg,min);
		char e=pos.GetNorS();
		if (e == 'N') s=1;
		if (e == 'S') s=-1;
		int ld = (int)round((min-floor(min*100)/100)*1000); // last digit of MM.MMM
		if (ld==3 && fix==1) min=min+1./3000; // when MM.MM3 AND not an ARF
		if (ld==7 && fix==1) min=min-1./3000; // when MM.MM7 AND not an ARF
		double degDec=s*(deg+min/60);
		//printf("Lat:%f E=%c S=%d Deg=%d Min=%f ld:%d\n", degDec, e, s, deg, min, ld);
		file << std::fixed << std::setprecision(6);
		file << degDec << ',';

		// Longitude, 'Lon' in CSV spec, expressed as real DDD.MMMMMM
		pos.GetLonDegMin(deg,min);
		e=pos.GetEorW();
		if (e == 'E') s=1;
		if (e == 'W') s=-1;
		ld = (int)round((min-floor(min*100)/100)*1000); // last digit of MM.MMM
		if (ld==3 && fix==1) min=min+1./3000; // when MM.MM3 AND not an ARF
		if (ld==7 && fix==1) min=min-1./3000; // when MM.MM7 AND not an ARF
		degDec=s*(deg+min/60);
		//printf("Lon:%f E=%c S=%d Deg=%d Min=%f ld:%d\n", degDec, e, s, deg, min, ld);
		file << degDec << ',';

		// Altitude, 'Elev' in CSV spec, must be feet without unit to be read by LNM
		double meters=w.GetAltitude();
		file << std::setprecision(0) << std::round(meters/0.3048) << ",";

		// Declination is ignored by CSV importers, leave empty
		file << ',';

		// Label/Tag is composed by Dir:N Len:N Freq:N to avoid lost of information
		if (w.IsAirfield()) {
			const Airfield& a((const Airfield&)w);

			// Runway direction, miss in CSV spec
			if (a.HasRunwayDir()) file << "Dir:" << std::setfill('0') << std::setw(3) << a.GetRunwayDir() << " ";

			// Runway length, miss in CSV spec
			if (a.HasRunwayLength()) file << "Len:" << a.GetRunwayLength() << "m ";

			// Radio frequency, miss in CSV spec
			if (a.HasRadioFrequency()) {
				file << "Freq:" << std::setprecision(3) << AirspaceConverter::FrequencyMHz(a.GetRadioFrequency()); // 3 decimals for Airports freq [MHz]
				if (a.HasOtherFrequency()) file << '-' << AirspaceConverter::FrequencyMHz(a.GetOtherFrequency());
			}
		} else {
			//file << ",,"; // Skip runway direction and length

			// Other frequency
			if (w.HasOtherFrequency()) {
				if (w.GetType() == Waypoint::WaypointType::NDB) file << "Freq:" << std::setprecision(2) << AirspaceConverter::FrequencykHz(w.GetOtherFrequency()); // 2 decimals for NDB freq [kHz]
				else if (w.GetType() == Waypoint::WaypointType::VOR) file << "Freq:" << std::setprecision(2) << AirspaceConverter::FrequencyMHz(w.GetOtherFrequency()); // 2 decimals for VOR freq [MHz]
				else file << "Freq:" << std::setprecision(2) << AirspaceConverter::FrequencyMHz(w.GetOtherFrequency()); // assuming all other VHF freq [MHz]
			}
		}
		file << ',';

		// Description, 'Desc' in CSV spec, redundant for VRP and IRP
		//if (!w.GetDescription().empty() && w.GetDescription().compare("VFR")!=0 && w.GetDescription().compare("IFR")!=0) file << '"' << w.GetDescription() << '"';
		if (!w.GetDescription().empty() && w.GetDescription().compare("VFR")!=0 && w.GetDescription().compare("IFR")!=0) file << w.GetDescription();
		file << ',';

		// Country code
		file << w.GetCountry() << ',';

		// Range is missing in source, leave empty
		//file << ',';

		// ModificationTime is ignored by CSV importers, leave empty
		//file << ',';

		// SourceFile is ignored by CSV importers, leave empty
		//file << ',';
		file << "\r\n";
	}

	file.close();
	return true;
}
