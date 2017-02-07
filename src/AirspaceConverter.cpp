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
#include "OpenAIPreader.h"
#include "Airspace.h"
#include "KMLwriter.h"
#include "PFMwriter.h"
#include "OpenAir.h"
#include "CUPreader.h"
#include "Waypoint.h"
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

std::function<void(const std::string&, const bool)> AirspaceConverter::LogMessage = DefaultLogMessage;
std::function<bool(const std::string&, const std::string&)> AirspaceConverter::cGPSmapper = Default_cGPSmapper;

const std::vector<std::string> AirspaceConverter::disclaimer = {
	"This file has been produced with: \"AirspaceConverter\" Version: " VERSION,
	"For info visit: http://www.alus.it/AirspaceConverter",
	"Copyrights(C) 2016-2017 Alberto Realis-Luc",
	"",
	"WARNING:",
	"This is a BETA version of AirspaceConverter!",
	"So beware that the output may contain errors!!!",
	"The only usage of this software is to test it; so, you, as tester user, are kindly requested to report any error or discrepancy found.",
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
	KMLwriter::ClearTerrainMaps();
	UnloadWaypoints();
}

void AirspaceConverter::DefaultLogMessage(const std::string& msgText, const bool isError) {
	(isError ? std::cerr : std::cout) << msgText << std::endl;
}

bool AirspaceConverter::Default_cGPSmapper(const std::string& polishFile, const std::string& outputFile) {
	LogMessage("Invoking cGPSmapper to make: " + outputFile, false);

	//TODO: add arguments to create files also for other software like Garmin BaseCamp
	const std::string cmd(boost::str(boost::format("cgpsmapper %1s -o %2s") %polishFile %outputFile));

	if(system(cmd.c_str()) == EXIT_SUCCESS) {
		std::remove(polishFile.c_str()); // Delete polish file
		return true;
	}

	LogMessage("ERROR: returned by cGPSmapper",true);
	return false;
}

std::istream& AirspaceConverter::SafeGetline(std::istream& is, std::string& line, bool& isCRLF) {
	line.clear();
	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();
	isCRLF = false;
	for(;;) {
		const int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return is;
		case '\r': // Beware that to detect the CR under Windows it is necessary to read the file in binary mode
			if(sb->sgetc() == '\n') {
				sb->sbumpc();
				isCRLF = true;
			}
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if(line.empty()) is.setstate(std::ios::eofbit);
			isCRLF = true; // no problem in this case
			return is;
		default:
			line += (char)c;
		}
	}
}

void AirspaceConverter::LoadAirspaces() {
	conversionDone = false;
	OpenAir openAir(airspaces);
	bool redOk(false);
	for (const std::string& inputFile : airspaceFiles) {
		const std::string ext(boost::filesystem::path(inputFile).extension().string());
		if(boost::iequals(ext, ".txt")) redOk = openAir.ReadFile(inputFile);
		else if (boost::iequals(ext, ".aip")) redOk = OpenAIPreader::ReadFile(inputFile, airspaces);

		// Guess a default output file name if still not defined by the user
		if (redOk && outputFile.empty())
			outputFile = boost::filesystem::path(inputFile).replace_extension(".kmz").string(); // Default output as KMZ
	}
	airspaceFiles.clear();
}

void AirspaceConverter::UnloadAirspaces() {
	conversionDone = false;
	airspaces.clear();
}

void AirspaceConverter::LoadTerrainRasterMaps() {
	conversionDone = false;
	for (const std::string& demFile : terrainRasterMapFiles) KMLwriter::AddTerrainMap(demFile);
	terrainRasterMapFiles.clear();
}

void AirspaceConverter::UnloadRasterMaps() {
	conversionDone = false;
	KMLwriter::ClearTerrainMaps();
}

void AirspaceConverter::LoadWaypoints() {
	conversionDone = false;
	for (const std::string& inputFile : waypointFiles) CUPreader::ReadFile(inputFile, waypoints);
	waypointFiles.clear();
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
	KMLwriter::SetDefaultTerrainAltitude(altMt);
}

double AirspaceConverter::GetDefaultTearrainAlt() const {
	return KMLwriter::GetDefaultTerrainAltitude();
}

bool AirspaceConverter::Convert() {
	conversionDone = false;
	if(outputFile.empty()) return false;

	// Determine what kind of output is requested
	OutputType outputType = AirspaceConverter::KMZ; // KMZ default
	std::string outputExt(boost::filesystem::path(outputFile).extension().string());
	if (!boost::iequals(outputExt, ".kmz")) {
		if(boost::iequals(outputExt, ".mp")) outputType = AirspaceConverter::Polish;
		else if(boost::iequals(outputExt, ".txt")) outputType = AirspaceConverter::OpenAir_Format;
		else if(boost::iequals(outputExt, ".img")) outputType = AirspaceConverter::Garmin;
		else {
			AirspaceConverter::LogMessage("ERROR: Output file extension/type unknown.",true);
			return false;
		}
	}
	
	switch (outputType) {
	case AirspaceConverter::KMZ:
		{
			KMLwriter writer;
			if (writer.WriteFile(outputFile, airspaces, waypoints)) {
				conversionDone = true;
				if(KMLwriter::GetNumOfRasterMaps() == 0) LogMessage("Warning: no raster terrain map loaded, used default terrain height for all applicable AGL points.", true);
				else if(!writer.WereAllAGLaltitudesCovered()) LogMessage("Warning: not all AGL altitudes were under coverage of the loaded terrain map(s).", true);
			}
		}
		break;
	case AirspaceConverter::OpenAir_Format:
		conversionDone = OpenAir(airspaces).WriteFile(outputFile);
		break;
	case AirspaceConverter::Polish:
		conversionDone = PFMwriter().WriteFile(outputFile, airspaces);
		break;
	case AirspaceConverter::Garmin: // For Garmin IMG will be necessary to call cGPSmapper
		{
			// First make the Polish file
			const std::string polishFile(boost::filesystem::path(outputFile).replace_extension(".mp").string());
			AirspaceConverter::LogMessage("Building Polish file: " + polishFile, false);
			if(!PFMwriter().WriteFile(polishFile, airspaces)) break;

			// Then call cGPSmapper
			conversionDone = cGPSmapper(polishFile, outputFile);
		}
		break;
	default:
		assert(false);
		break;
	}
	return conversionDone;
}

int AirspaceConverter::GetNumOfTerrainMaps() const {
	return KMLwriter::GetNumOfRasterMaps();
}
