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

#include "AirspaceConverter.h"
#include "Airspace.h"
#include "Waypoint.h"
#include "KML.h"
#include "OpenAir.h"
#include "SeeYou.h"
#include "OpenAIP.h"
#include "Polish.h"
#include <iostream>
#include <locale>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

std::function<void(const std::string&, const bool)> AirspaceConverter::LogMessage = DefaultLogMessage;
std::function<bool(const std::string&, const std::string&)> AirspaceConverter::cGPSmapper = Default_cGPSmapper;
std::string AirspaceConverter::cGPSmapperCommand = "cgpsmapper";

const std::vector<std::string> AirspaceConverter::disclaimer = {
	"This file has been produced with: \"AirspaceConverter\" Version: " VERSION,
	"For info visit: http://www.alus.it/AirspaceConverter",
	"Copyrights(C) 2016-2017 Alberto Realis-Luc",
	"",
	"WARNING:",
	"AirspaceConverter is an experimental software. So, please, be aware that the output may contain errors!",
	"The users are kindly requested to report any error or discrepancy found.",
	"",
	"Disclaimer:",
	"The author of AirspaceConverter assumes no liability at all for the previous, actual or future correctness, completeness, functionality or usability",
	"of the data provided in this file and the usage of AirspaceConverter. There exists no obligation at all for the author to continuously update",
	"or maintain the data provided. The airspace structure in this file and the data contained therein are only intended to serve as a means to facilitate",
	"familiarization with and to illustrate air space structure. This airspace structure file does not replace the pilot's obligation for preflight",
	"planning nor shall it be used as a means of support during flight. In particular, use of the this airspace structure file does not excuse the user",
	"from the responsibility to observe the current issue of any relevant AIP, AIP Supplements, NOTAM and AICs.",
	"The use of this airspace structure and/or waypoints file takes place only at the user's total own risk.",
	"Commercial use of the data provided via this airspace structure and/or waypoints file is strictly prohibited.",
	"The use of AirspaceConverter is only at complete user's own risk.",
	"Any commercial usage of AirspaceConverter is also strictly prohibited if not authorized by the author.",
	"",
	"Placemarks icons credits:",
	"Some of the placemark icons used for displaying the waypoints in Google Earth included in the produced KMZ file (and so used by this project) are",
	"coming from: \"Maps Icons Collection\" - https://mapicons.mapsmarker.com",
	"",
	"Error reports, complaints and suggestions please email to: info@alus.it",
};

AirspaceConverter::AirspaceConverter() :
	conversionDone(false) {
}

AirspaceConverter::~AirspaceConverter() {
	KML::ClearTerrainMaps();
	UnloadWaypoints();
}

void AirspaceConverter::DefaultLogMessage(const std::string& msgText, const bool isError) {
	(isError ? std::cerr : std::cout) << msgText << std::endl;
}

bool AirspaceConverter::Default_cGPSmapper(const std::string& polishFile, const std::string& outputFile) {
	LogMessage("Invoking cGPSmapper to make: " + outputFile, false);

	//TODO: add arguments to create files also for other software like Garmin BaseCamp
	const std::string cmd(boost::str(boost::format("%1s %2s -o %3s") %cGPSmapperCommand %polishFile %outputFile));
	LogMessage("Executing: " + cmd, false);
	if(system(cmd.c_str()) == EXIT_SUCCESS) {
		std::remove(polishFile.c_str()); // Delete polish file
		return true;
	}

	LogMessage("ERROR: returned by cGPSmapper.",true);
	return false;
}

void AirspaceConverter::SetIconsPath(const std::string& iconsPath) {
	KML::SetIconsPath(iconsPath);
}

std::istream& AirspaceConverter::SafeGetline(std::istream& is, std::string& line, bool& isCRLF) {
	line.clear();
	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();
	isCRLF = false;
	bool proceed(true);
	do {
		const int c = sb->sbumpc();
		switch (c) {
		case '\n':
			proceed = false;
			break;
		case '\r': // Beware that to detect the CR under Windows it is necessary to read the file in binary mode
			if(sb->sgetc() == '\n') {
				sb->sbumpc();
				isCRLF = true;
			}
			proceed = false;
			break;
		case EOF:
			// Also handle the case when the last line has no line ending
			if(line.empty()) is.setstate(std::ios::eofbit);
			isCRLF = true; // no problem in this case
			proceed = false;
			break;
		default:
			line += (char)c;
		}
	} while(proceed);
	return is;
}

AirspaceConverter::OutputType AirspaceConverter::DetermineType(const std::string& filename) {
	if (filename.empty()) return OutputType::KMZ_Format; // KMZ default
	OutputType outputType = OutputType::KMZ_Format; // KMZ default
	std::string outputExt(boost::filesystem::path(filename).extension().string());
	if (!boost::iequals(outputExt, ".kmz")) {
		if (boost::iequals(outputExt, ".txt")) outputType = OutputType::OpenAir_Format;
		else if (boost::iequals(outputExt, ".mp")) outputType = OutputType::Polish_Format;
		else if (boost::iequals(outputExt, ".img")) outputType = OutputType::Garmin_Format;
		else outputType = OutputType::Unknown_Format;
	}
	return outputType;
}

bool AirspaceConverter::PutTypeExtension(const OutputType type, std::string& filename) {
	if (filename.empty()) return false;
	boost::filesystem::path outputPath(filename);
	switch (type) {
	case OutputType::KMZ_Format:
		outputPath.replace_extension(".kmz");
		break;
	case OutputType::OpenAir_Format:
		outputPath.replace_extension(".txt");
		break;
	case OutputType::Polish_Format:
		outputPath.replace_extension(".mp");
		break;
	case OutputType::Garmin_Format:
		outputPath.replace_extension(".img");
		break;
	default:
		assert(false);
		/* no break */
	case OutputType::Unknown_Format:
		return false;
	}
	filename = outputPath.string();
	return true;
}

void AirspaceConverter::LoadAirspaces() {
	conversionDone = false;
	OpenAir openAir(airspaces);
	KML kml(airspaces, waypoints);
	int counter = 0;
	const size_t airspaceCounter = airspaces.size();
	for (const std::string& inputFile : airspaceFiles) {
		bool redOk(false);
		const std::string ext(boost::filesystem::path(inputFile).extension().string());
		if(boost::iequals(ext, ".txt")) redOk = openAir.Read(inputFile);
		else if (boost::iequals(ext, ".aip")) redOk = OpenAIP::Read(inputFile, airspaces);
		else if (boost::iequals(ext, ".kmz")) redOk = kml.ReadKMZ(inputFile);
		else if (boost::iequals(ext, ".kml")) redOk = kml.ReadKML(inputFile);
		if (redOk) counter++; // Count the files red correctly

		// Guess a default output file name if still not defined by the user
		if (redOk && outputFile.empty())
			outputFile = boost::filesystem::path(inputFile).replace_extension(".kmz").string(); // Default output as KMZ
	}
	airspaceFiles.clear();
	if (counter > 0) LogMessage(boost::str(boost::format("Red successfully %1d airspace definition(s) from %2d file(s).") %(airspaces.size() - airspaceCounter) %counter), false);
}

void AirspaceConverter::UnloadAirspaces() {
	conversionDone = false;
	airspaces.clear();
}

void AirspaceConverter::LoadTerrainRasterMaps() {
	conversionDone = false;
	int counter = 0;
	for (const std::string& demFile : terrainRasterMapFiles) if (KML::AddTerrainMap(demFile)) counter++;
	terrainRasterMapFiles.clear();
	if (counter > 0) LogMessage(boost::str(boost::format("Red successfully %1d terrain raster map file(s).") % counter), false);
}

void AirspaceConverter::UnloadRasterMaps() {
	conversionDone = false;
	KML::ClearTerrainMaps();
}

void AirspaceConverter::LoadWaypoints() {
	conversionDone = false;
	int counter = 0;
	const size_t wptCounter = waypoints.size();
	for (const std::string& inputFile : waypointFiles) {
		bool redOk = SeeYou::ReadFile(inputFile, waypoints);
		if (redOk) counter++;
		if (redOk && outputFile.empty()) outputFile = boost::filesystem::path(inputFile).replace_extension(".kmz").string(); // Default output as KMZ
	}
	waypointFiles.clear();
	if (counter > 0) LogMessage(boost::str(boost::format("Red successfully %1d waypoint(s) from %2d file(s).") % (waypoints.size() - wptCounter) %counter), false);
}

void AirspaceConverter::UnloadWaypoints() {
	conversionDone = false;
	for (const std::pair<const int, Waypoint*>& wpt : waypoints) delete wpt.second;
	waypoints.clear();
}

void AirspaceConverter::SetQNH(const double newQNHhPa) {
	Altitude::SetQNH(newQNHhPa);
}

double AirspaceConverter::GetQNH() const {
	return Altitude::GetQNH();
}

void AirspaceConverter::SetDefaultTearrainAlt(const double altMt) {
	KML::SetDefaultTerrainAltitude(altMt);
}

double AirspaceConverter::GetDefaultTearrainAlt() const {
	return KML::GetDefaultTerrainAltitude();
}

bool AirspaceConverter::Convert() {
	conversionDone = false;
	switch (GetOutputType()) {
	case OutputType::KMZ_Format:
		{
			KML writer(airspaces, waypoints);
			if (writer.Write(outputFile)) {
				conversionDone = true;
				if(KML::GetNumOfRasterMaps() == 0) LogMessage("Warning: no raster terrain map loaded, used default terrain height for all applicable AGL points.", true);
				else if(!writer.WereAllAGLaltitudesCovered()) LogMessage("Warning: not all AGL altitudes were under coverage of the loaded terrain map(s).", true);
			}
		}
		break;
	case OutputType::OpenAir_Format:
		conversionDone = OpenAir(airspaces).Write(outputFile);
		break;
	case OutputType::Polish_Format:
		conversionDone = Polish().Write(outputFile, airspaces);
		break;
	case OutputType::Garmin_Format: // For Garmin IMG will be necessary to call cGPSmapper
		{
			// First make the Polish file
			const std::string polishFile(boost::filesystem::path(outputFile).replace_extension(".mp").string());
			LogMessage("Building Polish file: " + polishFile, false);
			if(!Polish().Write(polishFile, airspaces)) break;

			// Then call cGPSmapper
			conversionDone = cGPSmapper(polishFile, outputFile);
		}
		break;
	default:
		LogMessage("ERROR: Output file extension/type unknown.", true);
		assert(false);
		break;
	}
	return conversionDone;
}

int AirspaceConverter::GetNumOfTerrainMaps() const {
	return KML::GetNumOfRasterMaps();
}

bool AirspaceConverter::ParseAltitude(const std::string& text, const bool isTop, Airspace& airspace) {
	if (text.empty()) return false;
	const std::string::size_type l = text.length();
	double value = 0;
	bool isFL = false;
	bool isAMSL = true;
	bool valueFound = false;
	bool typeFound = false;
	bool isInFeet = true;
	bool unitFound = false;
	bool isUnlimited = false;
	std::string::size_type s = 0;
	bool isNumber = isDigit(text.at(s));
	for (std::string::size_type i = 1; i < l; i++) {
		const char c = text.at(i);
		const bool isLast = (i == l - 1);
		const bool isSep = (c == ' ' || c == '=');
		if (isDigit(c) != isNumber || isSep || isLast) {
			const std::string str = isLast ? text.substr(s) : text.substr(s, i - s);
			if (isNumber) {
				if (!valueFound) {
					try {
						value = std::stod(str);
					}
					catch (...) {
						return false;
					}
					valueFound = true;
				}
				else return false;
			} else {
				if (!typeFound) {
					if (valueFound) {
						if (boost::iequals(str, "AGL") || boost::iequals(str, "AGND") || boost::iequals(str, "ASFC") || boost::iequals(str, "GND") || boost::iequals(str, "SFC")) {
							isAMSL = false;
							typeFound = true;
						} else if (boost::iequals(str, "MSL") || boost::iequals(str, "AMSL") || boost::iequals(str, "ALT")) typeFound = true;
						else if (!unitFound) {
							if (boost::iequals(str, "FT") || boost::iequals(str, "F")) unitFound = true;
							else if (boost::iequals(str, "M") || boost::iequals(str, "MT")) {
								isInFeet = false;
								unitFound = true;
							}
						}
					} else {
						if (boost::iequals(str, "FL")) {
							isFL = true;
							typeFound = true;
						} else if (boost::iequals(str, "GND") || boost::iequals(str, "SFC")) {
							isAMSL = false;
							typeFound = true;
							valueFound = true;
							unitFound = true;
						} else if (boost::iequals(str, "MSL") || boost::iequals(str, "AMSL")) {
							typeFound = true;
							valueFound = true;
							unitFound = true;
						} else if (boost::iequals(str, "UNLIM") || boost::iequals(str, "UNLIMITED") || boost::iequals(str, "UNL")) {
							typeFound = true;
							valueFound = true;
							unitFound = true;
							isUnlimited = true;
						}
					}
				} else if (!unitFound && !typeFound) return false;
			}
			if (valueFound && typeFound && unitFound) break;
			if (text.at(i) == ' ' || text.at(i) == '=') {
				if (++i < l) isNumber = isDigit(text.at(i));
			} else isNumber = !isNumber;
			s = i;
		}
	}
	if (!valueFound) return false;
	Altitude alt;
	if (isUnlimited) alt.SetUnlimited();
	else if (isFL) alt.SetFlightLevel((int)value);
	else if (isInFeet) alt.SetAltFt((int)value, isAMSL);
	else alt.SetAltMt(value, isAMSL);
	isTop ? airspace.SetTopAltitude(alt) : airspace.SetBaseAltitude(alt);
	return true;
}

