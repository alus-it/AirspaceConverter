//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2021 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "AirspaceConverter.h"
#include "Airspace.h"
#include "Waypoint.h"
#include "RasterMap.h"
#include "KML.h"
#include "OpenAir.h"
#include "SeeYou.h"
#include "OpenAIP.h"
#include "Polish.h"
#include "CSV.h"
#include <iostream>
#include <locale>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <map>
#include <tuple>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

// The HTTP client used to check for new version from Boost Beast is availble from version 1.70
#if BOOST_VERSION >= 107000
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#endif

#if BOOST_VERSION >= 106100
#include <boost/dll/runtime_symbol_info.hpp>

const std::string AirspaceConverter::basePath(boost::filesystem::path(boost::dll::program_location().parent_path()).string());
#else

// This is for older Linux distributions where boost::dll is not available
const std::string AirspaceConverter::basePath(boost::filesystem::exists("/usr/bin/airspaceconverter") ? "/usr/bin" : ".");
#endif

std::function<void(const std::string&)> AirspaceConverter::LogMessage = DefaultLogMessage;
std::function<void(const std::string&)> AirspaceConverter::LogWarning = DefaultLogWarning;
std::function<void(const std::string&)> AirspaceConverter::LogError = DefaultLogError;
std::function<bool(const std::string&, const std::string&)> AirspaceConverter::cGPSmapper = Default_cGPSmapper;

const std::vector<std::string> AirspaceConverter::disclaimer = {
	"This file has been produced with: \"AirspaceConverter\" Version: " VERSION,
	"For info visit: https://www.alus.it/AirspaceConverter",
	"Copyrights(C) 2016-2021 Alberto Realis-Luc, Valerio Messina",
	"",
	"WARNING:",
	"AirspaceConverter is an experimental software. So, please, be aware that the output may contain errors!",
	"The users are kindly requested to report any error or discrepancy found.",
	"",
	"Disclaimer:",
	"The authors of AirspaceConverter assume no liability at all for the previous, actual or future correctness, completeness, functionality or usability",
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
	"Error reports, complaints and suggestions please email to: info@alus.it",
};

std::vector<RasterMap*> AirspaceConverter::terrainMaps;
double AirspaceConverter::defaultTerrainAltitudeMt = 20;

const std::string AirspaceConverter::cGPSmapperCommand = Detect_cGPSmapperPath();


AirspaceConverter::AirspaceConverter() :
	conversionDone(false),
	processLineStrings(false) {
}

AirspaceConverter::~AirspaceConverter() {
	ClearTerrainMaps();
	UnloadWaypoints();
}

void AirspaceConverter::DefaultLogMessage(const std::string& text) {
	std::cout << text << std::endl;
}

void AirspaceConverter::DefaultLogWarning(const std::string& text) {
	std::clog << "Warning: " << text << std::endl;
}

void AirspaceConverter::DefaultLogError(const std::string& text) {
	std::cerr << "ERROR: " << text << std::endl;
}

const std::string AirspaceConverter::Detect_cGPSmapperPath() {
#ifdef __linux__ // If on Linux...
	if (boost::filesystem::exists("/usr/bin/cgpsmapper")) return "cgpsmapper"; // Check the default installation path then use the "cgpsmapper" command
	const std::string cGPSmapperCmd(basePath + "/cgpsmapper"); // Otherwise try to look in the same folder as the executable
	if (boost::filesystem::exists(cGPSmapperCmd)) return cGPSmapperCmd;
#elif _WIN32 // or if on Windows...
	const std::string cGPSmapperCmd(basePath + "\\cGPSmapper\\cgpsmapper.exe"); // Check the default installation path ... 
	if (boost::filesystem::exists(cGPSmapperCmd)) return cGPSmapperCmd; // then use it as command
#elif __APPLE__ // or if on macOS
	// Unfortunately there in no version of cGPSmapper available for macOS
#endif
	return ""; // cGPSmapper not found: return empty string 
}

bool AirspaceConverter::Default_cGPSmapper(const std::string& polishFile, const std::string& outputFile) {
	LogMessage("Invoking cGPSmapper to make: " + outputFile);

	//TODO: add arguments to create files also for other software like Garmin BaseCamp
	const std::string cmd(boost::str(boost::format("%1s \"%2s\" -o \"%3s\"") %cGPSmapperCommand %polishFile %outputFile));
	LogMessage("Executing: " + cmd);
	if(system(cmd.c_str()) == EXIT_SUCCESS) {
		std::remove(polishFile.c_str()); // Delete polish file
		return true;
	}

	LogError("returned by cGPSmapper.");
	return false;
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
		case EOF: // Also handle the case when the last line has no line ending
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
		else if (boost::iequals(outputExt, ".csv")) outputType = OutputType::CSV_Format;
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
	case OutputType::CSV_Format:
		outputPath.replace_extension(".csv");
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
	if (airspaceFiles.empty()) return;
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
		else {
			LogWarning("Unknown extension for airspace file: " + inputFile);
			continue;
		}

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
	LogMessage(boost::str(boost::format("Read %1d airspace definition(s) from %2d file(s).") %(airspaces.size() - initialAirspacesNumber) %airspaceFiles.size()));
	airspaceFiles.clear();
}

void AirspaceConverter::UnloadAirspaces() {
	conversionDone = false;
	airspaces.clear();
	outputFile.clear();
}

void AirspaceConverter::LoadTerrainRasterMaps() {
	if (terrainRasterMapFiles.empty()) return;
	conversionDone = false;
	int counter = 0;
	for (const std::string& demFile : terrainRasterMapFiles) if (AddTerrainMap(demFile)) counter++;
	terrainRasterMapFiles.clear();
	if (counter > 0) LogMessage(boost::str(boost::format("Read successfully %1d terrain raster map file(s).") % counter));
}

void AirspaceConverter::UnloadRasterMaps() {
	conversionDone = false;
	ClearTerrainMaps();
}

void AirspaceConverter::LoadWaypoints() {
	if (waypointFiles.empty()) return;
	conversionDone = false;
	int counter = 0;
	const size_t wptCounter = waypoints.size();
	SeeYou cu(waypoints);
	CSV csv(waypoints);
	OpenAIP openAIP(airspaces, waypoints);
	for (const std::string& inputFile : waypointFiles) {
		bool readOk(false);
		const std::string ext(boost::filesystem::path(inputFile).extension().string());
		if(boost::iequals(ext, ".cup")) readOk = cu.Read(inputFile);
		else if (boost::iequals(ext, ".csv")) readOk = csv.Read(inputFile);
		else if (boost::iequals(ext, ".aip")) readOk = openAIP.ReadWaypoints(inputFile);
		else {
			LogWarning("Unknown extension for waypoint file: " + inputFile);
			continue;
		}
		if (readOk) counter++;
		if (readOk && outputFile.empty()) outputFile = boost::filesystem::path(inputFile).replace_extension(".kmz").string(); // Default output as KMZ
	}
	waypointFiles.clear();
	if (counter > 0) LogMessage(boost::str(boost::format("Read successfully %1d waypoint(s) from %2d file(s).") % (waypoints.size() - wptCounter) %counter));
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

bool AirspaceConverter::AddTerrainMap(const std::string& filename) {
	RasterMap* pTerrainMap = new RasterMap();
	if (pTerrainMap == nullptr) return false;
	if (!pTerrainMap->Open(filename)) {
		delete pTerrainMap;
		return false;
	}
	terrainMaps.push_back(pTerrainMap);
	return true;
}

void AirspaceConverter::ClearTerrainMaps() {
	for (RasterMap* pTerreinMap : terrainMaps) if (pTerreinMap != nullptr) delete pTerreinMap;
	terrainMaps.clear();
}

bool AirspaceConverter::GetTerrainAltitudeMt(const double& lat, const double& lon, double& alt) {
	if (terrainMaps.empty()) return false; // no maps no party...
	const RasterMap* bestMap = terrainMaps.front();
	if (terrainMaps.size() > 1)
	{
		std::multimap<double, const RasterMap*> results; // preselected maps, minStepSize (kind of resolution) used as key
		double minStepSize = 8000; // 8000 it's just a quite big number which I like
		for (const RasterMap* pTerreinMap : terrainMaps) {
			if (pTerreinMap->PointIsInTerrainRange(lat, lon)) { // of course we want only maps covering our desired point!
				double stepSize = pTerreinMap->GetStepSize();
				if (stepSize < minStepSize) minStepSize = stepSize; // remember the best resolution
				results.insert(std::pair<double, const RasterMap*>(stepSize, pTerreinMap)); // maps indexed on resolution
			}
		}
		if (results.empty()) return false; // no results, the party is over ...
		if (results.size() == 1) bestMap = results.begin()->second; // only one, so that's easy
		else {
			double minLatDiff = 90; // to find a latitude difference more than 90 degrees should be quite challenging...
			const auto filtered = results.equal_range(minStepSize); // so now we have maps indexed on resolution and we even know the best resolution ...
			for (auto it = filtered.first; it != filtered.second; ++it) { // look for the map with our point at higer absolute latitudes (samples more dense on earth surface)
				double latDiff = lat >= 0 ? it->second->GetTop() - lat : lat - it->second->GetBottom();
				assert(latDiff >= 0);
				if (latDiff < minLatDiff) { // look for the minimum latitude difference with the proper N or S edge of the map
					minLatDiff = latDiff;
					bestMap = it->second;
				}
			}
		}
		assert(bestMap->PointIsInTerrainRange(lat, lon));
	}
	short altMt;
	if (bestMap->GetTerrainHeight(lat, lon, altMt)) {
		alt = altMt;
		return true;
	}
	return false;
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
				if(terrainMaps.empty()) LogWarning("no raster terrain map loaded, used default terrain height for all applicable AGL points.");
				else if(!writer.WereAllAGLaltitudesCovered()) LogWarning("not all AGL altitudes were under coverage of the loaded terrain map(s).");
			}
		}
		break;
	case OutputType::OpenAir_Format:

		conversionDone = OpenAir(airspaces).Write(outputFile);
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
			LogMessage("Building Polish file: " + polishFile);
			if(!Polish().Write(polishFile, airspaces)) break;

			// Then call cGPSmapper
			conversionDone = cGPSmapper(polishFile, outputFile);
		}
		break;
	case OutputType::CSV_Format:
		conversionDone = CSV(waypoints).Write(outputFile);
		break;
	default:
		LogError("Output file extension/type unknown.");
		assert(false);
		break;
	}
	return conversionDone;
}

bool AirspaceConverter::ConvertOpenAIPdir(const std::string openAIPdir) {
	if (openAIPdir.empty()) return false;
	const boost::filesystem::path openAIPpath(openAIPdir);
	if(!boost::filesystem::is_directory(openAIPpath)) {
		LogError("input openAIP airspace directory is not a valid directory: " + openAIPdir);
		return false;
	}
	UnloadAirspaces(); // make sure there is no airspace before to start...
	UnloadWaypoints(); // ... and also no waypoints

	// Build an index of openAIP contents for each country
	std::map<std::string,std::tuple<bool,bool,bool,bool>> aipFilesIndex;
	for (boost::filesystem::directory_iterator itr(openAIPpath); itr!=boost::filesystem::directory_iterator(); ++itr) {
		if (boost::filesystem::is_regular_file(itr->status()) && boost::filesystem::file_size(itr->path()) && boost::iequals(itr->path().extension().string(), ".aip")) {
			//const std::string openAIPfile(itr->path().relative_path().string());
			const std::string filename(itr->path().stem().string());
			if (filename.length() == 6 && filename.at(2)=='_') {
				const std::string countryCode(filename.substr(0,2));
				std::tuple<bool,bool,bool,bool> newContent(false,false,false,false); // 0:asp (airspace), 1:hot (hotspots), 2:nav (navaids), 3:wpt (airports)
				if (boost::ends_with(filename,"asp"))      std::get<0>(newContent) = true;
				else if (boost::ends_with(filename,"wpt")) std::get<3>(newContent) = true;
				else if (boost::ends_with(filename,"nav")) std::get<2>(newContent) = true;
				else if (boost::ends_with(filename,"hot")) std::get<1>(newContent) = true;
				else {
					LogWarning("not able to understand the content type from the name of openAIP file: " + filename);
					continue;
				}
				auto jtr = aipFilesIndex.find(countryCode);
				if (jtr == aipFilesIndex.end()) aipFilesIndex[countryCode] = newContent;
				else {
					std::tuple<bool,bool,bool,bool>& contents(jtr->second);
					std::get<0>(contents) = std::get<0>(contents) || std::get<0>(newContent);
					std::get<1>(contents) = std::get<1>(contents) || std::get<1>(newContent);
					std::get<2>(contents) = std::get<2>(contents) || std::get<2>(newContent);
					std::get<3>(contents) = std::get<3>(contents) || std::get<3>(newContent);
				}
			} else LogWarning("openAIP filename expected as <country code>_<content code> but found: " + filename);
		}
	}

	if (aipFilesIndex.empty()) {
		LogError("no .aip files found in directory: " + openAIPdir);
		return false;
	}

	for (const auto& record : aipFilesIndex) {
		const std::string& countryCode(record.first);
		const bool& asp(std::get<0>(record.second));
		//TODO: const bool& hot(std::get<1>(record.second));
		const bool& nav(std::get<2>(record.second));
		const bool& wpt(std::get<3>(record.second));
		std::string airfieldsFile;

		if (asp) {
			boost::filesystem::path aspPath(openAIPpath / std::string(countryCode + "_asp.aip"));
			AddAirspaceFile(aspPath.string());
			LoadAirspaces();

			// Make OpenAir airspace file
			outputFile = aspPath.replace_extension(".txt").string();
			Convert();
		}

		/* TODO: if (hot) {
			boost::filesystem::path navPath(openAIPpath / std::string(countryCode + "_hot.aip"));
			AddWaypointFile(navPath.string());
			LoadWaypoints(); // here load ONLY hotspots

			// Make SeeYou hotspots file
			outputFile = navPath.replace_extension(".cup").string();
			Convert();

			// Make LittleNavMap hotspots file
			outputFile = navPath.replace_extension(".csv").string();
			Convert();

			UnloadWaypoints();
		} */

		if (wpt) {
			boost::filesystem::path wptPath(openAIPpath / std::string(countryCode + "_wpt.aip"));
			airfieldsFile = wptPath.string(); // remember the airfields file
			AddWaypointFile(airfieldsFile);
			LoadWaypoints();

			// Make SeeYou airports file
			outputFile = wptPath.replace_extension(".cup").string();
			Convert();

			// Make LittleNavMap airports file
			outputFile = wptPath.replace_extension(".csv").string();
			Convert();
		}

		if (nav) {
			if (wpt) UnloadWaypoints(); // In case there were already airfield loaded unload them

			boost::filesystem::path navPath(openAIPpath / std::string(countryCode + "_nav.aip"));
			AddWaypointFile(navPath.string());
			LoadWaypoints(); // here load ONLY navaids

			// Make SeeYou navaids file
			outputFile = navPath.replace_extension(".cup").string();
			Convert();

			// Make LittleNavMap navaids file
			outputFile = navPath.replace_extension(".csv").string();
			Convert();
		}

		// In case airfields were unloaded reload them
		if (!airfieldsFile.empty()) {
			AddWaypointFile(airfieldsFile);
			LoadWaypoints();
		}

		// Make GoogleEarth KMZ file with all
		outputFile = boost::filesystem::path(openAIPpath / std::string(countryCode + ".kmz")).string();
		Convert();

		UnloadAirspaces(); //of course always unload everything before to load the next files
		UnloadWaypoints();
	}
	return true;
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
#ifdef __GNUC__
#if __GNUC__ > 4
	ss << "This file was created on: " << std::put_time(gmtime(&now), "%a %d %B %Y at %T UTC");
#else
	char dateString[40];
	struct tm *utc;
	utc = gmtime(&now);
	strftime(dateString, sizeof(dateString), "%a %d %B %Y at %T UTC", utc);
	ss << "This file was created on: " << dateString;
#endif
#else
#ifdef _WIN32
	struct tm utc;
	gmtime_s(&utc, &now);
	ss << "This file was created on: " << std::put_time(&utc, "%a %d %B %Y at %T UTC");
#endif
#endif
	return ss.str();
}

bool AirspaceConverter::CheckAirbandFrequency(const double& frequencyMHz, int& frequencyHz) {
	// Convert the frequency in Hz
	frequencyHz = (int)std::round(frequencyMHz * 1000000.0);

	// Check if the frequency is within the airband for communication (118.000 - 137.000 MHz) and if the frequency is a multiple of 1 kHz
	if (frequencyHz < 118000000 || frequencyHz > 137000000 || frequencyHz % 1000 != 0) {
		frequencyHz = 0;
		return false;
	}

	return true;
}

bool AirspaceConverter::CheckVORfrequency(const double& frequencyMHz, int& frequencyHz) {
	// Convert the frequency in Hz
	frequencyHz = (int)std::round(frequencyMHz * 1000000.0);
	
	// Check if the frequency is within VOR band (108.00 - 117.95 MHz) and if the frequency is a multiple of 50 kHz
	if (frequencyHz < 108000000 || frequencyHz > 117950000 || frequencyHz % 50000 != 0) {
		frequencyHz = 0;
		return false;
	}

	return true;
}

bool AirspaceConverter::CheckNDBfrequency(const double& frequencykHz, int& frequencyHz) {
	// Convert the frequency in Hz
	frequencyHz = (int)std::round(frequencykHz * 1000.0);

	// Check if the frequency is within the NDB band (190.0 - 1750.0 kHz) and if the frequency is a multiple of 0.1 kHz
	if (frequencyHz < 190000 || frequencyHz > 1750000 || frequencyHz % 100 != 0) {
		frequencyHz = 0;
		return false;
	}

	return true;
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
		LogMessage(boost::str(boost::format("Filtering airspaces... excluded: %1d, remaining: %2d") %(origAirspaces - GetNumOfAirspaces()) %GetNumOfAirspaces()));
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
		LogMessage(boost::str(boost::format("Filtering waypoints... excluded: %1d, remaining: %2d ") %(origWaypoints - GetNumOfWaypoints()) %GetNumOfWaypoints()));
	}

	return true;
}

void AirspaceConverter::DoNotCalculateArcsAndCirconferences(const bool doNotCalcArcs /*= true*/) {
	OpenAir::CalculateArcsAndCirconferences(!doNotCalcArcs);
}

void AirspaceConverter::SetOpenAirCoodinatesAutomatic() {
	OpenAir::SetCoordinateType(OpenAir::CoordinateType::AUTO);
}

void AirspaceConverter::SetOpenAirCoodinatesInDecimalMinutes() {
	OpenAir::SetCoordinateType(OpenAir::CoordinateType::DEG_DECIMAL_MIN);
}

void AirspaceConverter::SetOpenAirCoodinatesInSeconds() {
	OpenAir::SetCoordinateType(OpenAir::CoordinateType::DEG_MIN_SEC);
}

bool AirspaceConverter::VerifyAltitudeOnTerrainMap(const double& lat, const double& lon, float& alt, const bool& blankAltitude, const bool& altitudeParsed, const int& line, const bool isSpike) {
	double terrainAlt;
	if (GetTerrainAltitudeMt(lat, lon, terrainAlt)) {
		if (altitudeParsed) {
			const double delta = isSpike ? alt - terrainAlt : fabs(alt - terrainAlt); // Spike such as an antenna, tower, VOR, etc: consider higher theresold in this case
			if (isSpike ? delta > 100 || delta < 0 : terrainAlt > 5 ? delta > 10 : delta > 15) { // Consider bigger threshold if delta > 5 m (maybe it was really intended AMSL)
				LogWarning(boost::str(boost::format("on line %1d: detected altitude difference of: %2g m respect ground, using terrain altitude: %3g m") %line %delta %terrainAlt));
				alt = (float)terrainAlt;
			}
		} else {
			if (blankAltitude) LogWarning(boost::str(boost::format("on line %1d: blank elevation, using terrain altitude: %2g m") %line %terrainAlt));
			else LogWarning(boost::str(boost::format("on line %1d: invalid elevation, using terrain altitude: %2g m") %line %terrainAlt));
			alt = (float)terrainAlt;
		}
		return true;
	} 
	if (blankAltitude) LogWarning(boost::str(boost::format("on line %1d: blank elevation, waypoint out of loaded terrain maps: assuming AMSL") %line));
	else if (!altitudeParsed) LogWarning(boost::str(boost::format("on line %1d: invalid elevation, waypoint out of loaded terrain maps: assuming AMSL") %line));
	return false;
}

int AirspaceConverter::VersionToNumber(const std::string& versionString) {
	int versionNum = -1;
	if (versionString.length() == 5 && versionString.at(1) == '.' && versionString.at(3) == '.') {
		const char& major = versionString.at(0);
		const char& minor = versionString.at(2);
		const char& patch = versionString.at(4);
		if (isDigit(major) && isDigit(minor) && isDigit(patch)) versionNum = (major - '0') * 100 + (minor - '0') * 10 + (patch - '0');
	}
	return versionNum;
}

bool AirspaceConverter::CheckForNewVersion(int& versionDifference) {
	#if BOOST_VERSION >= 107000
	static const int runningVersionNumber = VersionToNumber(VERSION);
	assert(runningVersionNumber > 0 && runningVersionNumber < 1000);
	try {
		// The io_context is required for all I/O
		boost::asio::io_context ioc;

		// These objects perform our I/O
		boost::asio::ip::tcp::resolver resolver(ioc);
		boost::beast::tcp_stream stream(ioc);

		// Look up the domain name
		const std::string host = "alus.it";
		const auto results = resolver.resolve(host, "80");

		// Make the connection on the IP address we get from a lookup
		stream.connect(results);

		// Set up an HTTP GET request message (for HTTPS more libs would be required)
		boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/AirspaceConverter/lastVersion.txt", 11};
		req.set(boost::beast::http::field::host, host);
		req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		// Send the HTTP request to the remote host
		boost::beast::http::write(stream, req);

		// This buffer is used for reading and must be persisted
		boost::beast::flat_buffer buffer;

		// Declare a container to hold the response
		boost::beast::http::response<boost::beast::http::string_body> res;

		// Receive the HTTP response
		boost::beast::http::read(stream, buffer, res);

		// Gracefully close the socket
		boost::beast::error_code ec;
		stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

		// Not_connected happens sometimes: so don't bother reporting it.
		if (ec && ec != boost::beast::errc::not_connected) {
			throw boost::beast::system_error{ec};
		}
		
		// If we get here then the connection is closed gracefully

		// The content is the latest version string
		const std::string latestVersion = res.body();
		const int latestVersionNumber = VersionToNumber(latestVersion);
		if (latestVersionNumber > 0) {
			versionDifference = latestVersionNumber - runningVersionNumber;
			return true;
		}
	} catch (...) {
		// This can happen in many cases including when no Internet connection is availble; so: no need to bother the user
	}
	#endif
	return false;
}
