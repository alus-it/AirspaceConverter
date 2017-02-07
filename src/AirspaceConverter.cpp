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
	conversionDone(false),
	allAGLaltitudesCovered(false) {
}

AirspaceConverter::~AirspaceConverter() {
	KMLwriter::ClearTerrainMaps();
	UnloadWaypoints();
}

void AirspaceConverter::DefaultLogMessage(const std::string& msgText, const bool isError) {
	(isError ? std::cerr : std::cout) << msgText << std::endl;
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

bool AirspaceConverter::AddInputFile(const std::string& inputFile) {
	std::string ext(boost::filesystem::path(inputFile).extension().string());
	if (boost::iequals(ext, ".aip")) {
		openAIPinputFiles.push_back(inputFile);
		return true;
	}
	if (boost::iequals(ext, ".txt")) {
		openAirInputFiles.push_back(inputFile);
		return true;
	}
	return false;
}

void AirspaceConverter::LoadAirspaces() {
	conversionDone = false;
	for (const std::string& inputFile : openAIPinputFiles) OpenAIPreader::ReadFile(inputFile, airspaces);
	openAIPinputFiles.clear();
	OpenAir openAir(airspaces);
	for (const std::string& inputFile : openAirInputFiles) openAir.ReadFile(inputFile);
	openAirInputFiles.clear();
}

void AirspaceConverter::UnloadAirspaces() {
	conversionDone = false;
	airspaces.clear();
}

void AirspaceConverter::LoadTerrainRasterMaps() {
	conversionDone = false;
	for (const std::string& demFile : DEMfiles) KMLwriter::AddTerrainMap(demFile);
	DEMfiles.clear();
}

void AirspaceConverter::UnloadRasterMaps() {
	conversionDone = false;
	KMLwriter::ClearTerrainMaps();
}

void AirspaceConverter::LoadWaypoints() {
	conversionDone = false;
	for (const std::string& inputFile : CUPfiles) CUPreader::ReadFile(inputFile, waypoints);
	CUPfiles.clear();
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

bool AirspaceConverter::Convert(const std::string& outputFilename, AirspaceConverter::OutputType type) {
	conversionDone = false;
	outputFile = outputFilename;
	
	switch (type) {
	case AirspaceConverter::KMZ:
		{
			KMLwriter writer;
			if (writer.WriteFile(outputFile, airspaces, waypoints)) {
				conversionDone = true;
				allAGLaltitudesCovered = writer.WereAllAGLaltitudesCovered();
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

			//TODO: here we should call cGPSmapper but dependes on which SO we are....
			conversionDone = true;
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
