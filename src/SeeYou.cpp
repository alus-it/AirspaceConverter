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

#include "SeeYou.h"
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

SeeYou::SeeYou(std::multimap<int,Waypoint*>& waypointsMap):
	waypoints(waypointsMap) {
}

bool SeeYou::ParseLatitude(const std::string& text, double& lat) {
	const int len = (int)text.length();
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

bool SeeYou::ParseLongitude(const std::string& text, double& lon) {
	const int len = (int)text.length();
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

bool SeeYou::ParseAltitude(const std::string& text, float& alt) {
	alt = 0;
	int pos = (int)text.length() - 1;
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

bool SeeYou::ParseStyle(const std::string& text, int& type) {
	if (!text.empty()) try {
		type = std::stoi(text);
		if (type >= Waypoint::unknown && type < Waypoint::numOfWaypointTypes) return true;
	} catch(...) {}
	type = Waypoint::unknown;
	return false;
}

bool SeeYou::ParseRunwayDir(const std::string& text, int& dir) {
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

bool SeeYou::ParseRunwayLength(const std::string& text, int& length) {
	length = 0; // zero means invalid/unknown length
	if (text.empty()) return true; // empty field: declared as unknown
	int pos = (int)text.length() - 1;
	if (pos < 2) return false; // at least 3 chars: 2 digit and a letter for the unit: a runway of "9m" (2 char) would be not possible...
	bool feet = false, nauticalMiles = false, statuteMiles = false;
	if (text.back() == 'm' || text.back() == 'M') {
		if (text.at(pos-1) == 'n' || text.at(pos-1) == 'N') {
			pos--;
			nauticalMiles = true;
		}
	} else if ((text.at(pos-1) == 'm' || text.at(pos-1) == 'M') && (text.back() == 'l' || text.back() == 'L' || text.back() == 'i' || text.back() == 'I')) {
		pos--;
		statuteMiles = true;
	} else if ((text.at(pos-1) == 'f' || text.at(pos-1) == 'F') && (text.back() == 't' || text.back() == 'T')) {
		pos--;
		feet = true;
	} else return false; // Unable to parse unit
	try {
		double len = std::stod(text.substr(0,pos));
		if (len < 0) return false;
		if (nauticalMiles) len *= Geometry::NM2M;
		else if (statuteMiles) len *= Geometry::MI2M;
		else if (feet) len *= Altitude::FEET2METER;
		length = (int)std::round(len);
		return true;
	} catch (...) {}
	return false;
}

bool SeeYou::ParseAirfieldFrequencies(const std::string& text, int& freqHz, int& secondaryFreqHz) {
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

bool SeeYou::ParseOtherFrequency(const std::string& text, const int type, int& freqHz) {
	freqHz = 0;
	if (text.empty()) return true;
	if (type != Waypoint::WaypointType::VOR && type != Waypoint::WaypointType::NDB) return false; // This waypoint type is not supposed to have a frequency associated
	try {
		const double freq = std::stod(text);
		return type == Waypoint::WaypointType::VOR ? AirspaceConverter::CheckVORfrequency(freq,freqHz) : AirspaceConverter::CheckNDBfrequency(freq,freqHz);
	} catch(...) {}
	return false;
}

bool SeeYou::Read(const std::string& fileName) {
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
	float altitude = 0;
	const bool terrainMapsPresent(AirspaceConverter::GetNumOfTerrainMaps() > 0);

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
				sLine.find("name, code, country, lat, lon, elev, style, rwydir, rwylen, freq, desc") != std::string::npos ||
				sLine.find("name, code, country, lat, lon, elev, style, rwdir, rwlen, freq, desc") != std::string::npos)) continue;

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
		const std::string name = boost::trim_copy(*token);
		if (name.empty()) {
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: a name must be present: %2s") %linecount %sLine));
			continue;
		}

		// Code (short name)
		const std::string code = boost::trim_copy(*(++token));

		// Country code
		const std::string country = boost::trim_copy(*(++token));

		// Latitude
		if (!ParseLatitude(boost::trim_copy(*(++token)), latitude)) {
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: invalid latitude: %2s") %linecount %(*token)));
			continue;
		}

		// Longitude
		if (!ParseLongitude(boost::trim_copy(*(++token)), longitude)) {
			AirspaceConverter::LogError(boost::str(boost::format("on line %1d: invalid longitude: %2s") %linecount %(*token)));
			continue;
		}

		// Elevation
		const std::string elevationText(boost::trim_copy(*(++token)));
		const bool blankAltitude = elevationText.empty();
		const bool altitudePresent = blankAltitude ? false : ParseAltitude(elevationText, altitude);
		if (blankAltitude) altitude = 0;
		if (!altitudePresent && !blankAltitude && !terrainMapsPresent)
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid elevation: %2s, assuming AMSL") %linecount %elevationText));

		// Waypoint style
		if (!ParseStyle(boost::trim_copy(*(++token)),type))
			AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid waypoint style: %2s, assuming unknown") %linecount %(*token)));

		// Altitude verification against terrain raster map for only waypoints with null (or empty) altitude
		if (terrainMapsPresent && ((altitudePresent && altitude == 0) || !altitudePresent) && type > Waypoint::WaypointType::normal) { // Unknown and normal waypoints skipped
			double terrainAlt;
			if (AirspaceConverter::GetTerrainAltitudeMt(latitude, longitude, terrainAlt)) {
				if (altitudePresent) {
					if (fabs(terrainAlt - altitude) >= 5) { // Consider new altitude only if delta > 5 m (maybe it was really intended AMSL)
						AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: waypoint with null altitude, using terrain altitude: %2g m") %linecount %terrainAlt));
						altitude = (float)terrainAlt;
					}
				} else {
					if (blankAltitude)
						AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: blank elevation, using terrain altitude: %2g m") %linecount %terrainAlt));
					else
						AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid elevation: %2s, using terrain altitude: %2g m") %linecount %elevationText %terrainAlt));
					altitude = (float)terrainAlt;
				}
			} else {
				if (blankAltitude)
					AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: blank elevation, waypoint out of loaded terrain maps: assuming AMSL") %linecount));
				else if (!altitudePresent)
					AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid elevation: %2s, waypoint out of loaded terrain maps: assuming AMSL") %linecount %elevationText));
			}
		}

		// If it's an airfield...
		if(Waypoint::IsTypeAirfield((Waypoint::WaypointType)type)) {
			// Runway direction
			if (!ParseRunwayDir(boost::trim_copy(*(++token)),runwayDir))
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid runway direction: %2s") % linecount % (*token)));

			// Runway length
			if (!ParseRunwayLength(boost::trim_copy(*(++token)),runwayLength))
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid runway length: %2s") %linecount %(*token)));

			// Radio frequency
			if (!ParseAirfieldFrequencies(boost::trim_copy(*(++token)),radioFreq,altRadioFreq))
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid radio frequency for airfield: %2s") %linecount %(*token)));

			// Description
			std::string description = boost::trim_copy(*(++token));
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
			if (!ParseOtherFrequency(boost::trim_copy(*(++token)), type, radioFreq))
				AirspaceConverter::LogWarning(boost::str(boost::format("on line %1d: invalid frequency for non airfield waypoint: %2s") %linecount %(*token)));

			// Description
			std::string description = boost::trim_copy(*(++token));
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

bool SeeYou::Write(const std::string& fileName) {
	if (waypoints.empty()) {
		AirspaceConverter::LogMessage("SeeYou output: no waypoints, nothing to write");
		return false;
	}
	std::ofstream file;
	file.open(fileName, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!file.is_open() || file.bad()) {
		AirspaceConverter::LogError("Unable to open output file: " + fileName);
		return false;
	}
	AirspaceConverter::LogMessage("Writing SeeYou output file: " + fileName);

	// Write default CUP header on first line, and for compatibilty with "Strepla" do not write any disclaimer or comments 
	file << "name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc\r\n";

	// Go trough all waypoints
	for (const std::pair<int,Waypoint*>& pair : waypoints) {
		assert(pair.second != nullptr);
		const Waypoint& w(*pair.second);

		// Name is mandatory according to SeeYou specs
		if (w.GetName().empty()) {
			AirspaceConverter::LogWarning("skipping waypoint with long name empty: " + w.GetCode());
			continue;
		}

		// Long name
		file << '"' << w.GetName() << "\",";

		// Code "short name"
		if (!w.GetCode().empty()) file << '"' << w.GetCode() << '"';

		// Country code
		file << ',' << w.GetCountry() << ',';

		// Latitude
		int deg;
		double min; // the minutes must be expressed with 3 decimals so they have to be rounded on three decimals
		const Geometry::LatLon& pos(w.GetPosition());
		pos.GetLatDegMin(deg,min);
		file << std::setfill('0') << std::fixed << std::setprecision(3);
		file << std::setw(2) << deg << std::setw(6) << std::round(min*1000)/1000 << pos.GetNorS() << ',';

		// Longitude
		pos.GetLonDegMin(deg,min);
		file << std::setw(3) << deg << std::setw(6) << std::round(min*1000)/1000 << pos.GetEorW() << ',';

		// Altitude
		if (w.GetAltitude() != 0) file << std::setprecision(1) << std::round(w.GetAltitude()*10)/10 << "m,"; // round altitude in meters on one decimal
		else file << "0,";

		// Waypoint style
		file << (int)w.GetType() << ',';

		if (w.IsAirfield()) {
			const Airfield& a((const Airfield&)w);

			// Runway direction
			if (a.HasRunwayDir()) file << std::setw(3) << a.GetRunwayDir();
			file << ',';

			// Runway length
			if (a.HasRunwayLength()) file << a.GetRunwayLength() << 'm';
			file << ',';

			// Radio frequency
			if (a.HasRadioFrequency()) {
				file << std::setprecision(3) << AirspaceConverter::FrequencyMHz(a.GetRadioFrequency());
				if (a.HasOtherFrequency()) file << '-' << AirspaceConverter::FrequencyMHz(a.GetOtherFrequency());
			}
		} else {
			file << ",,"; // Skip runway length and direction

			// Other frequency
			if (w.HasOtherFrequency()) {
				if (w.GetType() == Waypoint::WaypointType::NDB) file << std::setprecision(1) << AirspaceConverter::FrequencykHz(w.GetOtherFrequency()); // 1 decimal for NDB freq [kHz]
				else if (w.GetType() == Waypoint::WaypointType::VOR) file << std::setprecision(2) << AirspaceConverter::FrequencyMHz(w.GetOtherFrequency()); // 2 decimals for VOR freq [MHz]
				else file << std::setprecision(3) << AirspaceConverter::FrequencyMHz(w.GetOtherFrequency()); // assuming all other VHF freq [MHz]
			}
		}
		file << ',';

		// Description
		if (!w.GetDescription().empty()) file << '"' << w.GetDescription() << '"';
		file << "\r\n";
	}

	file.close();
	return true;
}
