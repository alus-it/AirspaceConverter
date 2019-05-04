//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
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
#include <sstream>
#include <chrono>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

std::function<void(const std::string&, const bool)> AirspaceConverter::LogMessage = DefaultLogMessage;
std::function<bool(const std::string&, const std::string&)> AirspaceConverter::cGPSmapper = Default_cGPSmapper;
std::string AirspaceConverter::cGPSmapperCommand = "cgpsmapper";

const std::vector<std::string> AirspaceConverter::disclaimer = {
	"This file has been produced with: \"AirspaceConverter\" Version: " VERSION,
	"For info visit: http://www.alus.it/AirspaceConverter",
	"Copyrights(C) 2016-2019 Alberto Realis-Luc, Valerio Messina",
	"",
	"WARNING:",
	"AirspaceConverter is an experimental software. So, please, be aware that the output may contain errors!",
	"The users are kindly requested to report any error or discrepancy found.",
	"",
	"Disclaimer:",
	"The authors of AirspaceConverter assumes no liability at all for the previous, actual or future correctness, completeness, functionality or usability",
	"of the data provided in this file and the usage of AirspaceConverter. There exists no obligation at all for the authors to continuously update",
	"or maintain the data provided. The airspace structure in this file and the data contained therein are only intended to serve as a means to facilitate",
	"familiarization with and to illustrate air space structure. This airspace structure file does not replace the pilot's obligation for preflight",
	"planning nor shall it be used as a means of support during flight. In particular, use of the this airspace structure file does not excuse the user",
	"from the responsibility to observe the current issue of any relevant AIP, AIP Supplements, NOTAM and AICs.",
	"The use of this airspace structure and/or waypoints file takes place only at the user's total own risk.",
	"Commercial use of the data provided via this airspace structure and/or waypoints file is strictly prohibited.",
	"The use of AirspaceConverter is only at complete user's own risk.",
	"Any commercial usage of AirspaceConverter is also strictly prohibited if not authorized by its owner.",
	"",
	"Placemarks icons credits:",
	"Some of the placemark icons used for displaying the waypoints in Google Earth included in the produced KMZ file (and so used by this project) are",
	"coming from: \"Maps Icons Collection\" - https://mapicons.mapsmarker.com",
	"",
	"Error reports, complaints and suggestions please email to: info@alus.it",
};

AirspaceConverter::AirspaceConverter() :
	conversionDone(false),
	doNotCalculateArcs(false),
	writeCoordinatesAsDDMMSS(false),
	processLineStrings(false) {
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
		else if (boost::iequals(outputExt, ".cup")) outputType = OutputType::SeeYou_Format;
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
	case OutputType::SeeYou_Format:
		outputPath.replace_extension(".cup");
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

void AirspaceConverter::LoadAirspaces(const OutputType suggestedTypeForOutputFilename /* = OutputType::KMZ_Format */) {
	conversionDone = false;
	OpenAir openAir(airspaces);
	OpenAIP openAIP(airspaces, waypoints);
	KML kml(airspaces, waypoints);
	kml.ProcessLineStrings(processLineStrings);
	const size_t initialAirspacesNumber = airspaces.size(); // Airspaces originally already loaded
	for (const std::string& inputFile : airspaceFiles) {
		const std::string ext(boost::filesystem::path(inputFile).extension().string());
		if(boost::iequals(ext, ".txt")) openAir.Read(inputFile);
		else if (boost::iequals(ext, ".aip")) openAIP.ReadAirspaces(inputFile);
		else if (boost::iequals(ext, ".kmz")) kml.ReadKMZ(inputFile);
		else if (boost::iequals(ext, ".kml")) kml.ReadKML(inputFile);

		// Set (suggest) the output file name if still not defined by the user
		if (airspaces.size() > initialAirspacesNumber && outputFile.empty()) switch (suggestedTypeForOutputFilename) {
			default:
				assert(false);
				/* no break */
			case OutputType::KMZ_Format: // KMZ default extension
				outputFile = boost::filesystem::path(inputFile).replace_extension(".kmz").string();
				break;
			case OutputType::OpenAir_Format:
				outputFile = boost::filesystem::path(inputFile).replace_extension(".txt").string();
				break;
			case OutputType::Polish_Format:
				outputFile = boost::filesystem::path(inputFile).replace_extension(".mp").string();
				break;
			case OutputType::Garmin_Format:
				outputFile = boost::filesystem::path(inputFile).replace_extension(".img").string();
		}
	}
	LogMessage(boost::str(boost::format("Read %1d airspace definition(s) from %2d file(s).") %(airspaces.size() - initialAirspacesNumber) %airspaceFiles.size()), false);
	airspaceFiles.clear();
}

void AirspaceConverter::UnloadAirspaces() {
	conversionDone = false;
	airspaces.clear();
	outputFile.clear();
}

void AirspaceConverter::LoadTerrainRasterMaps() {
	conversionDone = false;
	int counter = 0;
	for (const std::string& demFile : terrainRasterMapFiles) if (KML::AddTerrainMap(demFile)) counter++;
	terrainRasterMapFiles.clear();
	if (counter > 0) LogMessage(boost::str(boost::format("Read successfully %1d terrain raster map file(s).") % counter), false);
}

void AirspaceConverter::UnloadRasterMaps() {
	conversionDone = false;
	KML::ClearTerrainMaps();
}

void AirspaceConverter::LoadWaypoints() {
	conversionDone = false;
	int counter = 0;
	const size_t wptCounter = waypoints.size();
	SeeYou cu(waypoints);
	for (const std::string& inputFile : waypointFiles) {
		const bool readOk = cu.Read(inputFile);
		if (readOk) counter++;
		if (readOk && outputFile.empty()) outputFile = boost::filesystem::path(inputFile).replace_extension(".kmz").string(); // Default output as KMZ
	}
	waypointFiles.clear();
	if (counter > 0) LogMessage(boost::str(boost::format("Read successfully %1d waypoint(s) from %2d file(s).") % (waypoints.size() - wptCounter) %counter), false);
}

void AirspaceConverter::UnloadWaypoints() {
	conversionDone = false;
	for (const std::pair<const int, Waypoint*>& wpt : waypoints) delete wpt.second;
	waypoints.clear();
	if (airspaces.empty()) outputFile.clear();
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
	assert(!outputFile.empty());
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
		conversionDone = OpenAir(airspaces, doNotCalculateArcs, writeCoordinatesAsDDMMSS).Write(outputFile);
		break;
	case OutputType::SeeYou_Format:
		conversionDone = SeeYou(waypoints).Write(outputFile);
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

bool AirspaceConverter::ConvertOpenAIPdir(const std::string openAIPdir) {
	if (openAIPdir.empty()) return false;
	boost::filesystem::path openAIPpath(openAIPdir);
	if(boost::filesystem::is_directory(openAIPpath)) {
		UnloadAirspaces(); // make sure there is no airspace before to start
		int counter(0);
		for (boost::filesystem::directory_iterator itr(openAIPpath); itr!=boost::filesystem::directory_iterator(); ++itr) {
			if (boost::filesystem::is_regular_file(itr->status()) && boost::filesystem::file_size(itr->path()) && boost::iequals(itr->path().extension().string(), ".aip")) {
				AddAirspaceFile(itr->path().relative_path().string());
				LoadAirspaces();
				Convert();
				boost::filesystem::path currentFilePath(itr->path());
				outputFile = currentFilePath.replace_extension(".txt").string();
				Convert();
				counter++;
				UnloadAirspaces(); //of course always unload airspace before to load the next file
			}
		}
		if (counter == 0) LogMessage("ERROR: no valid .aip files found in that directory.", true);
		else return true;
	} else LogMessage("ERROR: input openAIP airspace directory is not a valid directory.", true);
	return false;
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

std::string AirspaceConverter::GetCreationDateString() {
	const time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::stringstream ss;
	ss << "This file was created on: " << std::put_time(gmtime(&now), "%a %d %B %Y at %T UTC");
	return ss.str();
}

bool AirspaceConverter::FilterOnLatLonLimits(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon) {

	// Check if it is necessary to filter
	if (topLat == 90 && bottomLat == -90 && leftLon == -180 && rightLon == 180) return true;

	// Prepare the limits
	Geometry::Limits limits(topLat, bottomLat, leftLon, rightLon);

	// If no valid limits nothing to filter
	if (!limits.IsValid()) return false;

	// Filter airspace
	if (!airspaces.empty()) {
		const unsigned long origAirspaces(GetNumOfAirspaces());
		for (std::multimap<int, Airspace>::iterator it = airspaces.begin(); it != airspaces.end(); ) {
			if ((*it).second.IsWithinLimits(limits)) ++it;
			else it = airspaces.erase(it);
		}
		LogMessage(boost::str(boost::format("Filtering airspaces... excluded: %1d, remaining: %2d") %(origAirspaces - GetNumOfAirspaces()) %GetNumOfAirspaces()), false);
	}

	// Filter waypoints
	if (!waypoints.empty()) {
		const unsigned long origWaypoints(GetNumOfWaypoints());
		for (std::multimap<int, Waypoint*>::iterator it = waypoints.begin(); it != waypoints.end(); ) {
			Waypoint* w = (*it).second;
			if (limits.IsPositionWithinLimits(w->GetPosition())) ++it;
			else {
				it = waypoints.erase(it);
				delete(w);
			}
		}
		LogMessage(boost::str(boost::format("Filtering waypoints... excluded: %1d, remaining: %2d ") %(origWaypoints - GetNumOfWaypoints()) %GetNumOfWaypoints()), false);
	}

	return true;
}

