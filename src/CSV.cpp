//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Authors     : Valerio Messina <efa@iol.it>
//               Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2022 Valerio Messina, Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================
// CSV file format description as used by LittleNavMap, Plan-G, ...
// Are comma separated files, with lines like the following template:
//  Type,Name,Ident,Lat,Lon,Elev,Decl,Label,Desc,Country,Range,ModificationTime,SourceFile
// Note: Type can be: Airport, Airstrip, Bookmark, DME, Helipad, NDB, TACAN, VOR, VORDME, VORTAC, VRP, Waypoint
//       LNM2.4.5:Airport,Airstrip,Bookmark,Cabin,Closed,DME,Error,Flag,Helipad,Lighthouse,Location,Logbook,Marker,Mountain,NDB,Obstacle,POI,Pin,Seaport,TACAN,Unknown,VOR,VORDME,VORTAC,VRP,Waypoint
//       LNM2.6.x automatically detected comment line:
//        "Type,Name,Ident,Latitude,Longitude,Elevation,Magnetic Declination,Tags,Description,Region,Visible From,Last Edit,Import Filename"
// Note: Lat/Lon must be in DDD.MMMMMM format, Elev is in feet and unit must be omitted
// Note: Elevation is always feet and distances are always nautical miles
// Note: LNM fill the Declination field on export, not used when importing, leave empty
// Note: LNM fill the SourceFile field on export, ignore it when importing and take note of source file, leave empty
// https://www.littlenavmap.org/manuals/littlenavmap/release/2.4/en/USERPOINT.html#csv-data-format

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

bool CSV::ParseStyle(const std::string& text, int& type) {
	if (!text.empty()) try {
		//Airport, Airstrip, Bookmark, DME, Helipad, NDB, TACAN, VOR, VORDME, VORTAC, VRP, Waypoint
		//LNM2.4.5:Airport,Airstrip,Bookmark,Cabin,Closed,DME,Error,Flag,Helipad,Lighthouse,Location,Logbook,Marker,Mountain,NDB,Obstacle,POI,Pin,Seaport,TACAN,Unknown,VOR,VORDME,VORTAC,VRP,Waypoint
		/* 0123456789
			Airport,
			Airstrip,
			Bookmark,
			Cabin,
			Closed,
			DME,
			Error,
			Flag,
			Helipad,
			Lighthouse,
			Location,
			Logbook,
			Marker,
			Mountain,
			NDB,
			Obstacle,
			POI,
			Pin,
			Seaport,
			TACAN,
			Unknown,
			VOR,
			VORDME,
			VORTAC,
			VRP,
			Waypoint*/
		//Enum:        0,     1,             2,         3,          4,             5,         6,        7,      8,  9, 10,        11, 12,    13,    14,         15,    16,          17,                18
		//Enum:UNDEFINED,Normal,Airfield grass,Outlanding,Glider site,Airfield solid,Mount pass,Mount top,Antenna,VOR,NDB,Cool tower,Dam,Tunnel,Bridge,Power plant,Castle,Intersection,
		//Enum:unknown  ,normal,airfieldGrass ,outlanding,gliderSite ,airfieldSolid ,mtPass    ,mtTop    ,sender ,VOR,NDB,coolTower ,dam,tunnel,bridge,powerPlant ,castle,intersection,numOfWaypointTypes
		switch(text[0]) {
			case 'A': // Airport,Airstrip
				switch(text[3]) {
					case 'p': type=Waypoint::airfieldSolid; break;
					case 's': type=Waypoint::airfieldGrass; break;
					default:
						AirspaceConverter::LogWarning("point with unknown type: " + text);
						return false;
				}
				break;
			case 'B': // Bookmark
				type=Waypoint::unknown; break;
			case 'C': // Cabin,Closed
				switch(text[1]) {
					case 'a': type=Waypoint::unknown; break;
					case 'l': type=Waypoint::unknown; break;
					default:
						AirspaceConverter::LogWarning("point with unknown type: " + text);
						return false;
				}
				break;
			case 'D': // DME
				type=Waypoint::unknown; break;
			case 'E': // Error
				type=Waypoint::unknown; break;
			case 'F': // Flag
				type=Waypoint::unknown; break;
			case 'H': // Helipad
				type=Waypoint::unknown; break;
			case 'L': // Lighthouse,Location,Logbook
				switch(text[3]) {
					case 'h': type=Waypoint::unknown; break;
					case 'a': type=Waypoint::unknown; break;
					case 'b': type=Waypoint::unknown; break;
					default:
						AirspaceConverter::LogWarning("point with unknown type: " + text);
						return false;
				}
				break;
			case 'M': // Marker,Mountain
				switch(text[1]) {
					case 'a': type=Waypoint::unknown; break;
					case 'o': type=Waypoint::unknown; break;
					default:
						AirspaceConverter::LogWarning("point with unknown type: " + text);
						return false;
				}
				break;
			case 'N': // NDB
				type=Waypoint::NDB; break;
			case 'O': // Obstacle
				type=Waypoint::unknown; break;
			case 'P': // POI,Pin
				switch(text[1]) {
					case 'O': type=Waypoint::unknown; break;
					case 'i': type=Waypoint::unknown; break;
					default:
						AirspaceConverter::LogWarning("point with unknown type: " + text);
						return false;
				}
				break;
			case 'S': // Seaport
				type=Waypoint::unknown; break;
			case 'T': // TACAN
				type=Waypoint::VOR; break;
			case 'U': // Unknown
				type=Waypoint::unknown; break;
			case 'V': // VOR,VORDME,VORTAC,VRP
				switch(text[1]) {
					case 'O': 
						switch(text[3]) {
							case '\0': type=Waypoint::VOR; break;
							case 'D': type=Waypoint::VOR; break;
							case 'T': type=Waypoint::VOR; break;
							default:
								AirspaceConverter::LogWarning("point with unknown type: " + text);
								return false;
						}
						break;
					case 'R': type=Waypoint::intersection; break;
					default:
						AirspaceConverter::LogWarning("point with unknown type: " + text);
						return false;
				}
				break;
			case 'W': // Waypoint
				type=Waypoint::castle; break;
			default:
				AirspaceConverter::LogWarning("point with unknown type: " + text);
				return false;
		} // switch
		if (type >= Waypoint::unknown && type < Waypoint::numOfWaypointTypes) return true;
	} catch(...) {}
	type = Waypoint::unknown;
	return false;
}

bool CSV::ParseLatitude(const std::string& text, double& lat) {
	const size_t len = text.length();
	if(len < 1) return false;
	try {
		lat = std::stod(text);
	} catch(...) {
		return false;
	}
	return Geometry::LatLon::IsValidLat(lat);
}

bool CSV::ParseLongitude(const std::string& text, double& lon) {
	const size_t len = text.length();
	if(len < 1) return false;
	try {
		lon = std::stod(text);
	} catch (...) {
		return false;
	}
	return Geometry::LatLon::IsValidLon(lon);
}

bool CSV::ParseAltitude(const std::string& text, float& alt) {
	alt = 0;
	size_t pos = text.length() - 1;
	if(pos == 0 && text.front()=='0') return true; // empty ==> 0m
	if(pos<1) return false;
	bool feet = false;
	if(text.back() == 't' || text.back() == 'T') pos--;
	switch(text.at(pos)) {
		case 'm':
		case 'M':
			break;
		case 'f':
		case 'F':
		default:
			feet = true;
	}
	try {
		alt = std::stof(text);
		if(feet) alt *= (float)Altitude::FEET2METER;
		return true;
	} catch(...) {}
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

// Type,Name,Ident,Lat,Lon,Elev,Decl,Label,Desc,Country,Range,ModificationTime,SourceFile
bool CSV::Read(const std::string& fileName) {
	std::ifstream input(fileName, std::ios::binary);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogError("Unable to open CSV input file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Reading CSV file: " + fileName);
	int linecount = 0;
	std::string sLine;
	bool isCRLF = false, CRLFwarningGiven = false, firstWaypointFound = false;

	int type;
	double latitude, longitude;
	float altitude = 0;
	const bool terrainMapsPresent(AirspaceConverter::GetNumOfTerrainMaps() > 0);

	while (!input.eof() && input.good()) {

		// Get the line
		AirspaceConverter::SafeGetline(input, sLine, isCRLF);
		linecount++;

		// Verify line ending
		if (!CRLFwarningGiven && !isCRLF) {
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: not valid Windows style end of line (expected CR LF).") % linecount));

			// CSV files may contain thousands of WPs we don't want to print this warning all the time
			CRLFwarningGiven = true;
		}

		// Directly skip empty lines
		if (sLine.empty()) continue;

		// Skip eventual header
		if (!firstWaypointFound && (
				sLine.find("Type,Name,Ident,Latitude,Longitude,Elevation,Magnetic Declination,Tags,Description,Region,Visible From,Last Edit,Import Filename") != std::string::npos ||
				sLine.find("Type,Name,Ident,Lat,Lon,Elev,Decl,Label,Desc,Country,Range,ModificationTime,SourceFile") != std::string::npos)) continue;

		// Remove front spaces
		boost::algorithm::trim_left(sLine);

		// If it was a line with only spaces skip it
		if (sLine.empty()) continue;

		// Then directly skip full comment line
		//if (sLine.front() == '*') continue;

		// Remove back spaces
		boost::algorithm::trim_right(sLine);

		// Skip too short lines
		if (sLine.size() <= 4) { // At least 4 commas should be there
			AirspaceConverter::LogError(boost::str(boost::format("line %1d is too short to contain anything useful: %2s") %linecount %sLine));
			continue;
		}

		// Tokenize with quotes
		boost::tokenizer<boost::escaped_list_separator<char> > tokens(sLine); // default separator:',', default quote:'"', default escape char:'\'
		if (std::distance(tokens.begin(),tokens.end()) < 10) { // We expect at least 10 fields
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: expected 10 fields: %2s") %linecount %sLine));
			continue;
		}

		boost::tokenizer<boost::escaped_list_separator<char> >::iterator token=tokens.begin();

		// Waypoint style
		if (!ParseStyle(boost::trim_copy(*token),type)) // check & fix: ParseStyle()
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid waypoint style: %2s, assuming unknown") %linecount %(*token)));

		// Long name
		const std::string name = boost::trim_copy(*(++token));
		if (name.empty()) {
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: a name must be present: %2s") %linecount %sLine));
			continue;
		}

		// Code (short name)
		const std::string code = boost::trim_copy(*(++token));

		// Latitude
		if (!ParseLatitude(boost::trim_copy(*(++token)), latitude)) { // check & fix: ParseLatitude()
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: invalid latitude: %2s") %linecount %(*token)));
			continue;
		}

		// Longitude
		if (!ParseLongitude(boost::trim_copy(*(++token)), longitude)) { // check & fix: ParseLongitude()
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: invalid longitude: %2s") %linecount %(*token)));
			continue;
		}

		// Elevation
		const std::string elevationText(boost::trim_copy(*(++token)));
		const bool blankAltitude = elevationText.empty();
		if (blankAltitude) altitude = 0;
		const bool altitudeParsed = blankAltitude ? false : ParseAltitude(elevationText, altitude); // check & fix: ParseAltitude()
		
		if (!altitudeParsed && !blankAltitude && !terrainMapsPresent)
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid elevation: %2s, assuming AMSL") %linecount %elevationText));

		// Altitude verification against terrain raster map
		if (terrainMapsPresent && type > Waypoint::WaypointType::normal) // Unknown and normal waypoints skipped
			AirspaceConverter::VerifyAltitudeOnTerrainMap(latitude,longitude,altitude,blankAltitude,altitudeParsed,linecount,type >= Waypoint::WaypointType::mtTop && type <= Waypoint::WaypointType::dam);

		// If it's an airfield...
		if(Waypoint::IsTypeAirfield((Waypoint::WaypointType)type)) { // check & fix: Waypoint.h:IsTypeAirfield()

			token++; // Skip Declination
#if 0
			//maybe in Label/Tag field: runwayDir,runwayLength,radioFreq
			//int runwayDir=-1, runwayLength=-1, radioFreq=-1, altRadioFreq=-1;

			// Runway direction
			if (!ParseRunwayDir(boost::trim_copy(*(++token)),runwayDir)) // check & fix: ParseRunwayDir()
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid runway direction: %2s") % linecount % (*token)));

			// Runway length
			if (!ParseRunwayLength(boost::trim_copy(*(++token)),runwayLength)) // check & fix: ParseRunwayLength()
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid runway length: %2s") %linecount %(*token)));

			// Radio frequency
			if (!ParseAirfieldFrequencies(boost::trim_copy(*(++token)),radioFreq,altRadioFreq)) // check & fix: ParseAirfieldFrequencies()
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid radio frequency for airfield: %2s") %linecount %(*token)));
#endif
			token++; // Skip as now Label/Tag(runway direction, length and radio freq)

			// Description
			std::string description = boost::trim_copy(*(++token));

			// Country code
			const std::string country = boost::trim_copy(*(++token));

			assert(token != tokens.end());

			// Build the airfield (for now without runway dir and length and radio freq)
			Airfield* airfield = new Airfield(name, code, country, latitude, longitude, altitude, type, description);
#if 0
			if (altRadioFreq > 0) {
				assert(radioFreq > 0);
				if (altRadioFreq == radioFreq) AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: skipping repeated secondary radio frequency for airfield.") %linecount));
				else airfield->SetOtherFrequency(altRadioFreq);
			}
#endif

			// Add it to the multimap
			waypoints.insert(std::pair<int, Waypoint*>(type, (Waypoint*)airfield));

		} else { // If it's NOT an airfield...
			token++; // Skip Declination
			token++;// Skip as now Label/Tag (runway direction, length and radio freq)

#if 0
			// Frequency may be used for VOR and NDB
			if (!ParseOtherFrequency(boost::trim_copy(*(++token)), type, radioFreq)) // check & fix: ParseOtherFrequency()
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid frequency for non airfield waypoint: %2s") %linecount %(*token)));
#endif

			// Description
			std::string description = boost::trim_copy(*(++token));
			if (description.length()==0) {
				if (type==Waypoint::castle) description.assign("IFR");
				if (type==Waypoint::intersection) description.assign("VFR");
			}

			// Country code
			const std::string country = boost::trim_copy(*(++token));

			assert(token != tokens.end());

			// Build the waypoint
			Waypoint* waypoint = new Waypoint(name, code, country, latitude, longitude, altitude, type, description);

#if 0
			if (radioFreq > 0) waypoint->SetOtherFrequency(radioFreq);
#endif

			// Add it to the multimap
			waypoints.insert(std::pair<int, Waypoint*>(type, waypoint));
		}

		// Make sure that at this point we already found a valid waypoint so the header is not anymore expected
		if (!firstWaypointFound) firstWaypointFound = true;

	} // while
	return true;
}

// Type,Name,Ident,Lat,Lon,Elev,Decl,Label,Desc,Country,Range,ModificationTime,SourceFile
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

	// LNMv2.4.6 Write default CSV header on first line, and for compatibilty do not write any other comments
	//file << "Type,Name,Ident,Lat,Lon,Elev,Decl,Label,Desc,Country,Range,ModificationTime,SourceFile\r\n";

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
		file << std::setprecision(0) << std::round(w.GetAltitude()/0.3048) << ",";

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
