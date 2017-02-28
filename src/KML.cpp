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

#include "KML.h"
#include "Airspace.h"
#include "RasterMap.h"
#include "AirspaceConverter.h"
#include "Waypoint.h"
#include "Airfield.h"
#include "Geometry.h"
#include <zip.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/tokenizer.hpp>

const std::string KML::colors[][2] = {
	{ "509900ff", "7f9900ff" }, //CLASSA
	{ "50cc0000", "7fcc0000" }, //CLASSB
	{ "50cc3399", "7fcc3399" }, //CLASSC
	{ "50ff9900", "7fff9900" }, //CLASSD
	{ "50339900", "7f339900" }, //CLASSE
	{ "503399ff", "7f3399ff" }, //CLASSF
	{ "50ff99ff", "7fff99ff" }, //CLASSG
	{ "500000FF", "7F0000FF" }, //DANGER
	{ "500000FF", "7F0000FF" }, //PROHIBITED
	{ "50FF0080", "7FFF0080" }, //RESTRICTED
	{ "501947ff", "7f1947ff" }, //CTR
	{ "50ffdd01", "7fffdd01" }, //TMA
	{ "50000000", "7fd4d4d4" }, //TMZ
	{ "50000000", "7fd4d4d4" }, //RMZ
	{ "40000000", "7fd4d4d4" }, //FIR
	{ "40000000", "7fd4d4d4" }, //UIR
	{ "40000000", "7fd4d4d4" }, //OTH
	{ "ff00aa55", "7f00e2e2" }, //GLIDING
	{ "400000FF", "7fd4d4d4" }, //NOGLIDER
	{ "403399ff", "7fd4d4d4" }, //WAVE
	{ "40000000", "7fd4d4d4" }, //UNKNOWN
	{ "40000000", "7fd4d4d4" }  //UNDEFINED
};

const std::string KML::airfieldColors[][2] = {
	{ "", "" }, //UNDEFINED
	{ "", "" }, //Normal
	{ "4b14F064", "3214F064" }, //AirfieldGrass
	{ "4b143C64", "37143C64" }, //Outlanding
	{ "4bFAFAFA", "37FAFAFA" }, //GliderSite
	{ "4b6E6E6E", "376E6E6E" }, //AirfieldSolid
};

const std::string KML::waypointIcons[] = {
	"undefined.png", //UNDEFINED
	"normal.png", //Normal
	"airfieldgrass.png", //AirfieldGrass
	"outlanding.png", //Outlanding
	"glidersite.png", //GliderSite
	"airfieldsolid.png", //AirfieldSolid
	"mountpass.png", // Mt pass
	"mounttop.png", // Mt top
	"sender.png", // Sender
	"vor.png", // VOR
	"ndb.png", // NDB
	"cooltower.png", // CoolTower
	"dam.png", // Dam
	"tunnel.png", // Tunnel
	"bridge.png", // Bridge
	"powerplant.png", // PowerPlant
	"castle.png", // Castle
	"intersection.png" // Intersection
};

std::vector<RasterMap*> KML::terrainMaps;
double KML::defaultTerrainAltitudeMt = 20.842;
std::string KML::iconsPath = "/usr/share/airspaceconverter/icons/"; // Default installed Linux location

KML::KML(std::multimap<int, Airspace>& airspacesMap, std::multimap<int, Waypoint*>& waypointsMap):
		airspaces(airspacesMap),
		waypoints(waypointsMap),
		allAGLaltitudesCovered(true),
		folderCategory(Airspace::Type::UNDEFINED) {
}

bool KML::AddTerrainMap(const std::string& filename) {
	RasterMap* pTerrainMap = new RasterMap();
	if (pTerrainMap == nullptr) return false;
	if (!pTerrainMap->Open(filename)) {
		delete pTerrainMap;
		return false;
	}
	terrainMaps.push_back(pTerrainMap);
	return true;
}

void KML::ClearTerrainMaps() {
	for (RasterMap* pTerreinMap : terrainMaps) if (pTerreinMap != nullptr) delete pTerreinMap;
	terrainMaps.clear();
}

bool KML::GetTerrainAltitudeMt(const double& lat, const double& lon, double& alt) {
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

void KML::WriteHeader(const bool airspacePresent, const bool waypointsPresent) {
	assert(airspacePresent || waypointsPresent);
	outputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<!--\n";
	for(const std::string& line: AirspaceConverter::disclaimer) outputFile << line << "\n";
	outputFile << "-->\n"
		<< "<kml xmlns = \"http://www.opengis.net/kml/2.2\">\n"
		<< "<Document>\n"
		<< "<open>true</open>\n";
		if (airspacePresent) for (int t = Airspace::CLASSA; t <= Airspace::UNDEFINED; t++) {
			outputFile << "<Style id = \"Style" << Airspace::CategoryName((Airspace::Type)t) << "\">\n"
				<< "<LineStyle>\n"
				<< "<color>" << colors[t][0] << "</color>\n"
				<< "<width>1.5</width>\n"
				<< "</LineStyle>\n"
				<< "<PolyStyle>\n"
				<< "<color>" << colors[t][1] << "</color>\n"
				<< "</PolyStyle>\n"
				<< "</Style>\n";
		}
		if (waypointsPresent) for (int t = Waypoint::normal; t < Waypoint::numOfWaypointTypes; t++) {
			outputFile << "<Style id = \"Style" << Waypoint::TypeName((Waypoint::WaypointType)t) << "\">\n"
				<< "<IconStyle>\n"
				<< "<Icon>\n"
				<< "<href>icons/" << waypointIcons[t] <<"</href>\n"
				<< "</Icon>\n"
				<< "</IconStyle>\n";
			if (Waypoint::IsTypeAirfield((Waypoint::WaypointType)t))
				outputFile << "<LineStyle>\n"
					<< "<color>" << airfieldColors[t][0] << "</color>\n"
					<< "<width>1.5</width>\n"
					<< "</LineStyle>\n"
					<< "<PolyStyle>\n"
					<< "<color>" << airfieldColors[t][1] << "</color>\n"
					<< "</PolyStyle>\n";
			outputFile << "</Style>\n";
		}
}

void KML::OpenPlacemark(const Airspace& airspace) {
	outputFile << "<Placemark>\n"
		<< "<name>" << airspace.GetName() << "</name>\n"
		<< "<styleUrl>#Style" << airspace.GetCategoryName() << "</styleUrl>\n"
		<< "<visibility>" << (airspace.IsVisibleByDefault() ? 1 : 0) << "</visibility>\n"
		<< "<ExtendedData>\n"
		<< "<SchemaData>\n"
		<< "<SimpleData name=\"Name\">" << airspace.GetName() << "</SimpleData>\n"
		<< "<SimpleData name=\"Category\">" << (airspace.GetType() <= Airspace::CLASSG ? ("Class " + airspace.GetCategoryName()) : airspace.GetCategoryName() ) << "</SimpleData>\n"
		<< "<SimpleData name=\"Top\">" << airspace.GetTopAltitude().ToString() << "</SimpleData>\n"
		<< "<SimpleData name=\"Base\">" << airspace.GetBaseAltitude().ToString() << "</SimpleData>\n"
		<< "</SchemaData>\n"
		<< "</ExtendedData>\n";
}

void KML::OpenPlacemark(const Waypoint* waypoint) {
	const bool isAirfield = waypoint->IsAirfield();
	const int altMt = waypoint->GetAltitude();
	const int altFt = (int)round(altMt / Altitude::FEET2METER);
	outputFile << "<Placemark>\n"
		<< "<name>" << waypoint->GetName() << "</name>\n"
		<< "<styleUrl>#Style" << waypoint->GetTypeName() << "</styleUrl>\n";
	outputFile << "<visibility>" << (isAirfield ? 1 : 0) << "</visibility>\n"
		<< "<ExtendedData>\n"
		<< "<SchemaData>\n"
		<< "<SimpleData name=\"Name\">" << waypoint->GetName() << "</SimpleData>\n"
		<< "<SimpleData name=\"Type\">" << waypoint->GetTypeName() << "</SimpleData>\n"
		<< "<SimpleData name=\"Code\">" << waypoint->GetCode() << "</SimpleData>\n"
		<< "<SimpleData name=\"Country\">" << waypoint->GetCountry() << "</SimpleData>\n"
		<< "<SimpleData name=\"Altitude\">" << altMt << " m - " << altFt << " ft" << "</SimpleData>\n";
	if(isAirfield) {
		const Airfield* airfield = (const Airfield*)waypoint;
		outputFile << "<SimpleData name=\"Runway direction\">" << (airfield->GetRunwayDir() != -1 ? std::to_string(airfield->GetRunwayDir()) + " deg" : "UNKNOWN") << "</SimpleData>\n"
		<< "<SimpleData name=\"Runway length\">" << (airfield->GetRunwayLength() != -1 ? std::to_string(airfield->GetRunwayLength()) + " m" : "UNKNOWN") << "</SimpleData>\n"
		<< "<SimpleData name=\"Radio frequency\">" << (airfield->GetRadioFrequency().empty() ? "UNKNOWN" : airfield->GetRadioFrequency() + " MHz") << "</SimpleData>\n";
	}
	outputFile << "<SimpleData name=\"Description\">" << waypoint->GetDescription() << "</SimpleData>\n"
		<< "</SchemaData>\n"
		<< "</ExtendedData>\n";
}

void KML::OpenPolygon(const bool extrude, const bool absolute) {
	outputFile << "<Polygon>\n";
	if (extrude) outputFile << "<extrude>1</extrude>\n";
	outputFile << "<altitudeMode>" << (absolute ? "absolute" : "relativeToGround") << "</altitudeMode>\n"
		<< "<outerBoundaryIs>\n"
		<< "<LinearRing>\n"
		<< "<coordinates>\n";
}

void KML::ClosePolygon() {
	outputFile << "</coordinates>\n"
		<< "</LinearRing>\n"
		<< "</outerBoundaryIs>\n"
		<< "</Polygon>\n";
}

void KML::WriteBaseOrTop(const Airspace& airspace, const Altitude& alt, const bool extrudeToGround /*= false*/) {
	OpenPolygon(extrudeToGround, alt.IsAMSL());
	double altitude = alt.GetAltMt();
	for (const Geometry::LatLon& p : airspace.GetPoints()) outputFile << p.Lon() << "," << p.Lat() << "," << altitude << "\n";
	ClosePolygon();
}

void KML::WriteBaseOrTop(const Airspace& airspace, const std::vector<double>& altitudesAmsl) {
	OpenPolygon(false, true);
	assert(airspace.GetNumberOfPoints() == altitudesAmsl.size());
	for (unsigned int i = 0; i < altitudesAmsl.size(); i++) {
		const Geometry::LatLon p = airspace.GetPointAt(i);
		outputFile << p.Lon() << "," << p.Lat() << "," << altitudesAmsl.at(i) << "\n";
	}
	ClosePolygon();
}

void KML::WriteSideWalls(const Airspace& airspace) {
	assert(airspace.GetTopAltitude().IsAMSL() == airspace.GetBaseAltitude().IsAMSL());

	// Build closing wall between last and first point
	OpenPolygon(false, airspace.GetBaseAltitude().IsAMSL());
	double top = airspace.GetTopAltitude().GetAltMt();
	double base = airspace.GetBaseAltitude().GetAltMt();
	double lon1 = airspace.GetLastPoint().Lon();
	double lat1 = airspace.GetLastPoint().Lat();
	double lon2 = airspace.GetFirstPoint().Lon();
	double lat2 = airspace.GetFirstPoint().Lat();
	outputFile << lon1 << "," << lat1 << "," << top << "\n"
		<< lon2 << "," << lat2 << "," << top << "\n"
		<< lon2 << "," << lat2 << "," << base << "\n"
		<< lon1 << "," << lat1 << "," << base << "\n"
		<< lon1 << "," << lat1 << "," << top << "\n";
	ClosePolygon();

	// Build all other walls from first, point to point, until the last one
	for (unsigned int i = 0; i < airspace.GetNumberOfPoints() - 1; i++) {
		OpenPolygon(false, airspace.GetBaseAltitude().IsAMSL());
		airspace.GetPointAt(i).GetLatLon(lat1, lon1);
		airspace.GetPointAt(i + 1).GetLatLon(lat2, lon2);
		outputFile << lon1 << "," << lat1 << "," << top << "\n"
			<< lon2 << "," << lat2 << "," << top << "\n"
			<< lon2 << "," << lat2 << "," << base << "\n"
			<< lon1 << "," << lat1 << "," << base << "\n"
			<< lon1 << "," << lat1 << "," << top << "\n";
		ClosePolygon();
	}
}

void KML::WriteSideWalls(const Airspace& airspace, const std::vector<double>& altitudesAmsl) {
	assert(airspace.GetTopAltitude().IsAMSL() != airspace.GetBaseAltitude().IsAMSL());
	OpenPolygon(false, true);
	assert(airspace.GetNumberOfPoints() == altitudesAmsl.size());

	// Build closing wall between last and first point
	const bool isBase = airspace.GetBaseAltitude().IsAGL();
	double top1 = isBase ? airspace.GetTopAltitude().GetAltMt() : altitudesAmsl.back();
	double base1 = isBase ? altitudesAmsl.back() : airspace.GetBaseAltitude().GetAltMt();
	double top2 = isBase ? airspace.GetTopAltitude().GetAltMt() : altitudesAmsl.front();
	double base2 = isBase ? altitudesAmsl.front() : airspace.GetBaseAltitude().GetAltMt();
	double lon1 = airspace.GetLastPoint().Lon();
	double lat1 = airspace.GetLastPoint().Lat();
	double lon2 = airspace.GetFirstPoint().Lon();
	double lat2 = airspace.GetFirstPoint().Lat();
	outputFile << lon1 << "," << lat1 << "," << top1 << "\n"
		<< lon2 << "," << lat2 << "," << top2 << "\n"
		<< lon2 << "," << lat2 << "," << base2 << "\n"
		<< lon1 << "," << lat1 << "," << base1 << "\n"
		<< lon1 << "," << lat1 << "," << top1 << "\n";
	ClosePolygon();

	// Build all other walls from first, point to point, until the last one
	for (unsigned int i = 0; i < airspace.GetNumberOfPoints() - 1; i++) {
		OpenPolygon(false, true);
		top1 = isBase ? airspace.GetTopAltitude().GetAltMt() : altitudesAmsl.at(i);
		base1 = isBase ? altitudesAmsl.at(i) : airspace.GetBaseAltitude().GetAltMt();
		top2 = isBase ? airspace.GetTopAltitude().GetAltMt() : altitudesAmsl.at(i+1);
		base2 = isBase ? altitudesAmsl.at(i+1) : airspace.GetBaseAltitude().GetAltMt();
		airspace.GetPointAt(i).GetLatLon(lat1, lon1);
		airspace.GetPointAt(i + 1).GetLatLon(lat2, lon2);
		outputFile << lon1 << "," << lat1 << "," << top1 << "\n"
			<< lon2 << "," << lat2 << "," << top2 << "\n"
			<< lon2 << "," << lat2 << "," << base2 << "\n"
			<< lon1 << "," << lat1 << "," << base1 << "\n"
			<< lon1 << "," << lat1 << "," << top1 << "\n";
		ClosePolygon();
	}
}

bool KML::Write(const std::string& filename) {
	
	// Verify presence of waypoints and airspaces
	const bool airspacesPresent = !airspaces.empty();
	const bool waypointsPresent = !waypoints.empty();
	if((!airspacesPresent && !waypointsPresent) || filename.empty()) return false;
	
	// The file must be a KMZ
	if (!boost::iequals(boost::filesystem::path(filename).extension().string(), ".kmz")) {
		AirspaceConverter::LogMessage("ERROR: Expected KMZ extension but found: " + boost::filesystem::path(filename).extension().string(), true);
		return false;
	}

	// Prepare pathname to the KML doc.kml; KMZ files should have the KML file name named as "doc.kml"
	const std::string fileKML(boost::filesystem::path(boost::filesystem::path(filename).parent_path() / boost::filesystem::path("doc.kml")).string());

	// Make sure the file is not already open
	if (outputFile.is_open()) outputFile.close();
	
	// Open the output file
	outputFile.open(fileKML, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!outputFile.is_open() || outputFile.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open output file: " + filename, true);
		return false;
	}
	AirspaceConverter::LogMessage("Writing output file: " + fileKML, false);

	// Assume all points have AGL altitude covered (no point processed yet)
	allAGLaltitudesCovered = true;

	// Write KML header
	WriteHeader(airspacesPresent, waypointsPresent);

	// If there are waypoints
	if (waypointsPresent) {
		
		// If airspaces and waypoints are both present prepare a folder to group all the waypoints
		if (airspacesPresent && waypointsPresent) 
			outputFile << "<Folder>\n"
				"<name>Waypoints</name>\n"
				"<visibility>1</visibility>\n"
				"<open>true</open>\n";

		// For each waypoint type
		for (int t = Waypoint::normal; t < Waypoint::numOfWaypointTypes; t++) {

			// First verify if there are waypoints of that kind
			if (waypoints.count(t) == 0) continue;

			const bool isAirfield = Waypoint::IsTypeAirfield((Waypoint::WaypointType)t);

			// Prepare the folder
			outputFile << "<Folder>\n"
				"<name>" << Waypoint::TypeName((Waypoint::WaypointType)t) << "</name>\n"
				"<visibility>" << (isAirfield ? 1 : 0) <<"</visibility>\n"
				"<open>false</open>\n";
			
			const auto filtered = waypoints.equal_range(t);
			for (auto it = filtered.first; it != filtered.second; ++it) {
				//const Waypoint* w = it->second;
				const Waypoint* w = it->second;

				// Open placemark
				OpenPlacemark(w);

				// Flag to remember if the runway perimeter has been drawn
				bool airfieldDrawn = false;

				int dir = -1;
				
				// If it is an airfield draw an estimation of the runway perimeter
				if (isAirfield) {
					
					// Get the airfield
					const Airfield* a = (const Airfield*)w;
					
					// Get its rinway length and direction
					const int leng = a->GetRunwayLength();
					dir = a->GetRunwayDir();

					// If they are valid...
					if (leng > 0 && dir >= 0) {

						// Calculate the runway perimeter
						std::vector<Geometry::LatLon> airfieldPerimeter;
						if (Geometry::CalcAirfieldPolygon(a->GetLatitude(), a->GetLongitude(), leng, dir, airfieldPerimeter)) {
							
							// Open a multigeometry with a polygon clamped onto the ground
							outputFile << "<MultiGeometry>\n"
								<< "<Polygon>\n"
								//<< "<altitudeMode>clampToGround</altitudeMode>\n" //this should be the default
								<< "<outerBoundaryIs>\n"
								<< "<LinearRing>\n"
								<< "<coordinates>\n";

							// Add the four points
							for (const Geometry::LatLon& p : airfieldPerimeter)
								outputFile << p.Lon() << "," << p.Lat() << "," << a->GetAltitude() << "\n";
							
							// Close the perimeter re-adding the first point 
							outputFile << airfieldPerimeter.front().Lon() << "," << airfieldPerimeter.front().Lat() << "," << a->GetAltitude() << "\n";

							// Close the polygon
							ClosePolygon();

							// The airfield perimeter has been drawn
							airfieldDrawn = true;
						}
					}
				}

				// Draw the waypoint marker
				outputFile << "<Point>\n"
					<< "<extrude>0</extrude>\n"
					<< "<altitudeMode>" << (t != Waypoint::normal ? "clampToGround" : "absolute") << "</altitudeMode>\n" // Except "normal" are all objects on the ground
					<< "<coordinates>" << w->GetLongitude() << "," << w->GetLatitude() << "," << w->GetAltitude() << "</coordinates>\n"
					<< "</Point>\n";

				// If the perimeter was drawn the the multigeometry have to be closed
				if (airfieldDrawn) outputFile << "</MultiGeometry>\n";
				
				// If there is a valid direction set the orientation of the airport icon as the runway
				if (dir > 0)
					outputFile << "<Style>\n"
						<< "<IconStyle>\n"
						<< "<heading>" << dir << "</heading>\n"
						<< "</IconStyle>\n"
						<< "</Style>\n";
				
				// Close the placemark
				outputFile << "</Placemark>\n";
			}

			// Close the category
			outputFile << "</Folder>\n";
		} // for each category

		// Close waypoints folder
		if (airspacesPresent && waypointsPresent) outputFile << "</Folder>\n";
	} // if airspaces

	// If there are airspaces
	if(airspacesPresent) {

		// If airspaces and waypoints are both present prepare a folder to group all the airspace
		if (airspacesPresent && waypointsPresent)
			outputFile << "<Folder>\n"
				"<name>Airspace</name>\n"
				"<visibility>1</visibility>\n"
				"<open>true</open>\n";

		// For each airspace category
		for (int t = Airspace::CLASSA; t <= Airspace::UNDEFINED; t++) {

			// First verify if there are airspaces of that class
			if (airspaces.count(t) == 0) continue;

			// Prepare the folder
			outputFile << "<Folder>\n"
				"<name>" << Airspace::CategoryName((Airspace::Type)t) << "</name>\n"
				"<visibility>" << (Airspace::CategoryVisibleByDefault((Airspace::Type)t) ? 1 : 0) <<"</visibility>\n"
				"<open>false</open>\n";

			const auto filtered = airspaces.equal_range(t);
			for (auto it = filtered.first; it != filtered.second; ++it) {

				const Airspace& a = it->second;

				assert(a.GetNumberOfPoints() > 3);
				assert(a.GetFirstPoint()==a.GetLastPoint());

				OpenPlacemark(a);

				if (a.IsGNDbased() || a.IsMSLbased()) WriteBaseOrTop(a, a.GetTopAltitude(), true); // then that's easy!
				else { // otherwise we have to abuse KML which is not properly done to draw middle air airspaces
					outputFile << "<MultiGeometry>\n";

					if (a.GetTopAltitude().IsAMSL() == a.GetBaseAltitude().IsAMSL()) { // same reference, still doable

						// Top
						WriteBaseOrTop(a, a.GetTopAltitude());

						// Base
						WriteBaseOrTop(a, a.GetBaseAltitude());

						// Sides
						WriteSideWalls(a);
					}
					else { // base and top altitudes not on the same reference: so find all absolute altitudes!
						double altitudeAGLmt = (a.GetBaseAltitude().IsAGL() ? a.GetBaseAltitude() : a.GetTopAltitude()).GetAltMt();

						// Try to get terrein altitude then add the AGL altitude to get AMSL altitude
						std::vector<double> amslAltitudesMt;
						for (const Geometry::LatLon& p : a.GetPoints()) {
							double terrainHeightMt = defaultTerrainAltitudeMt;
							allAGLaltitudesCovered = GetTerrainAltitudeMt(p.Lat(), p.Lon(), terrainHeightMt) && allAGLaltitudesCovered;
							amslAltitudesMt.push_back(terrainHeightMt + altitudeAGLmt);
						}

						// Top or base, the one that is already defined as AMSL
						WriteBaseOrTop(a, a.GetTopAltitude().IsAMSL() ? a.GetTopAltitude() : a.GetBaseAltitude());

						// The other one where the altitude of the points has been reobtained as AMSL
						WriteBaseOrTop(a, amslAltitudesMt);

						// Sides, where the points with altitude AGL were reobtained as AMSL
						WriteSideWalls(a, amslAltitudesMt);
					}
					outputFile << "</MultiGeometry>\n";
				}
				outputFile << "</Placemark>\n";
			}

			// Close category folder
			outputFile << "</Folder>\n";
		} // for each category

		// Close airspaces folder
		if (airspacesPresent && waypointsPresent) outputFile << "</Folder>\n";
	} // if airspaces

	outputFile << "</Document>\n"
		<< "</kml>\n";
	outputFile.close();

	// Compress (ZIP) and do make the KMZ file
	AirspaceConverter::LogMessage("Compressing into KMZ: " + filename, false);

	// To avoid problems it is better to delete the KMZ file if already existing, user has already been warned
	if (boost::filesystem::exists(filename)) std::remove(filename.c_str()); // Delete KMZ file

	// Open the ZIP file
	int error = 0;
	zip* archive = zip_open(filename.c_str(), ZIP_CREATE, &error);
	if (error) {
		AirspaceConverter::LogMessage("ERROR: Could not open or create archive: " + filename, true);
		return false;
	}

	// Create source buffer from KML file
	assert(boost::filesystem::path(fileKML).filename().string() == "doc.kml");
	zip_source* source = zip_source_file(archive, fileKML.c_str(), 0, 0);
	if (source == nullptr) { // "failed to create source buffer. " << zip_strerror(archive)
		// Discard zip file. In case ZIP_FL_OVERWRITE is not defined we are using an older libzib version such as 0.10.1, so we have to use the older functions
#ifdef ZIP_FL_OVERWRITE
		zip_discard(archive);
#else
		zip_close(archive);
#endif
		AirspaceConverter::LogMessage("ERROR: Failed to create zip source buffer to read: " + fileKML, true);
		return false;
	}

	// Add the buffer as KLM file in the ZIP
#ifdef ZIP_FL_OVERWRITE
	int index = (int)zip_file_add(archive, "doc.kml", source, ZIP_FL_OVERWRITE);
#else
	int index = (int)zip_add(archive, "doc,kml", source);
#endif
	if (index < 0) { // "failed to add file to archive. " << zip_strerror(archive)
#ifdef ZIP_FL_OVERWRITE
		zip_discard(archive);
#else
		zip_close(archive);
#endif
		zip_source_free(source); // The sorce buffer have to be freed in this case
		AirspaceConverter::LogMessage("ERROR: While compressing, failed to add: doc.kml", true);
		return false;
	}

	// If it is necessary to add also the icons
	if (!waypoints.empty()) {
		std::string path(iconsPath);

		// Check if the configured icons path exits, user may be wrong...
		if (!boost::filesystem::exists(path)) {
			// Don't be so inflexible... Please try again in the current directory...
			path = std::string("./icons/");
			if (!boost::filesystem::exists(path)) path = std::string("./");
		}
		for (int i = Waypoint::undefined; i < Waypoint::numOfWaypointTypes; i++) {
			// Get the icon PNG filename and prepare the path in the ZIP and the path from current dir
			const std::string iconPath = path + waypointIcons[i];

			// Check if we can get that PNG file
			if (!boost::filesystem::exists(iconPath)) {
				AirspaceConverter::LogMessage("ERROR: Unable to find icon PNG file: " + iconPath, true);
				continue;
			}

			// Create source buffer from KML file
			source = zip_source_file(archive, (iconPath).c_str(), 0, 0);
			if (source == nullptr) { // "failed to create source buffer. " << zip_strerror(archive)
#ifdef ZIP_FL_OVERWRITE
				zip_discard(archive);
#else
				zip_close(archive);
#endif
				AirspaceConverter::LogMessage("ERROR: Failed to create zip source buffer to read: " + iconPath, true);
				return false;
			}

			// Add the buffer as PNG file in the ZIP
			const std::string iconFile = "icons/" + waypointIcons[i];
#ifdef ZIP_FL_OVERWRITE
			index = (int)zip_file_add(archive, iconFile.c_str(), source, ZIP_FL_OVERWRITE);
#else
			index = (int)zip_add(archive, iconFile.c_str(), source);
#endif
			if (index < 0) { // "failed to add file to archive. " << zip_strerror(archive)
#ifdef ZIP_FL_OVERWRITE
				zip_discard(archive);
#else
				zip_close(archive);
#endif
				zip_source_free(source); // The sorce buffer have to be freed in this case
				AirspaceConverter::LogMessage("ERROR: While compressing, failed to add: " + iconFile, true);
				return false;
			}
		}
	}

	// Close the zip
	if (zip_close(archive) == 0) {
		std::remove(fileKML.c_str()); // Delete KML file
		return true;
	}
	AirspaceConverter::LogMessage("ERROR: While finalizing the archive.", true);
	return false;
}

bool KML::ReadKMZ(const std::string& filename) {
	// Open the ZIP file
	int error = 0;
	zip* archive = zip_open(filename.c_str(), 0, &error);
	if (error) {
		AirspaceConverter::LogMessage("ERROR: Could not open KMZ file: " + filename, true);
		return false;
	}

	// Get the number of files in the ZIP
	const long nFiles = (long)zip_get_num_entries(archive, 0);
	if(nFiles < 1) {
		AirspaceConverter::LogMessage("ERROR: KMZ file seems empty or not valid: " + filename, true);
		zip_close(archive);
		return false;
	}

	AirspaceConverter::LogMessage("Opened KMZ file: " + filename, false);

	std::string extractedKmlFile;
	struct zip_stat sb;

	// Iterate trough the contents: look for the first KML file in the root of the ZIP file
	for (long i=0; i<nFiles; i++) {
		if (zip_stat_index(archive, i, 0, &sb) != 0) {
			AirspaceConverter::LogMessage("ERROR: while reading KMZ, unable to get details of a file in the ZIP.", true);
			continue;
		}
		int len = (int)strlen(sb.name);

		// Skip dirs
		if (sb.name[len - 1] == '/') continue;

		// Skip empty files
		if(sb.size == 0) continue;

		boost::filesystem::path zippedFilePath(sb.name);

		// Skip files not in the root of the ZIP
		if (!zippedFilePath.parent_path().string().empty()) continue;

		// Skip non KML files
		if (!boost::iequals(zippedFilePath.extension().string(),".kml")) continue;

		struct zip_file* zf = zip_fopen_index(archive, i, 0);
		if (zf == nullptr) {
			AirspaceConverter::LogMessage("ERROR: while extracting, unable to open KML file from KMZ: " + filename, true);
			continue;
		}

		// Prepare path and name of the kml file
		extractedKmlFile = boost::filesystem::path(boost::filesystem::path(filename).parent_path() / boost::filesystem::path(sb.name)).string();

		// To avoid problems it is better to delete the KML file if already existing, user has already been warned
		if (boost::filesystem::exists(extractedKmlFile)) std::remove(extractedKmlFile.c_str()); // Delete KML file

		// Open the file to be extracted: open new file
		std::ofstream kmlFile;
		kmlFile.open(extractedKmlFile, std::ios::out | std::ios::trunc | std::ios::binary);
		if (!kmlFile.is_open() || kmlFile.bad()) {
			AirspaceConverter::LogMessage("ERROR: While extracting KML file, unable to write: " + extractedKmlFile, true);
			zip_fclose(zf);
			zip_close(archive);
			return false;
		}

		// Read from the ZIP and write to the extracted file
		char buf[8000]; // 8000, the size of this read buffer, is just quite big number which I like
		unsigned long sum = 0;
		while (sum < sb.size) {
			len = (int)zip_fread(zf, buf, 8000);
			if (len < 0) {
				AirspaceConverter::LogMessage("ERROR: While extracting KML file, unable read compressed data from: " + filename, true);
				kmlFile.close();
				zip_fclose(zf);
				zip_close(archive);
				return false;
			}
			kmlFile.write(buf, len);
			sum += len;
		}

		// Close all
		kmlFile.close();
		zip_fclose(zf);

		AirspaceConverter::LogMessage("Extracted KML file: " + std::string(sb.name), false);

		// If we arrived at this point we assume that we just found and correctly extracted the KML file and so we don't need to go further in the KMZ
		break;
	}

	// Close the ZIP archive
	zip_close(archive);

	// No KML... no party...
	if(extractedKmlFile.empty()) return false;

	// So then ... let's try to read the KML file
	bool retValue = ReadKML(extractedKmlFile);

	// Delete the, probably huge, KML file because it already compressed inside the KMZ
	std::remove(extractedKmlFile.c_str());

	return retValue;
}

bool KML::ProcessFolder(const boost::property_tree::ptree& folder, const int upperCategory) {
	
	// Try to guess the category from the name of folder
	const std::string categoryName = folder.get<std::string>("name");
	int thisCategory = Airspace::Type::UNDEFINED;
	unsigned int first = (unsigned int)categoryName.find('(');
	if (first != std::string::npos) {
		unsigned int last = (unsigned int)categoryName.find(')');
		if (last != std::string::npos && first < last) {
			const std::string shortCategory = categoryName.substr(first+1, last - first - 1);
			if (shortCategory.length() == 1) {
				switch (shortCategory.at(0)) {
				case 'A': thisCategory = Airspace::Type::CLASSA; break;
				case 'B': thisCategory = Airspace::Type::CLASSB; break;
				case 'C': thisCategory = Airspace::Type::CLASSC; break;
				case 'D': thisCategory = (categoryName == "Danger areas (D)") ? Airspace::Type::DANGER : Airspace::Type::CLASSD; break;
				case 'E': thisCategory = Airspace::Type::CLASSE; break;
				case 'F': thisCategory = Airspace::Type::CLASSF; break;
				case 'G': thisCategory = Airspace::Type::CLASSG; break;
				case 'P': thisCategory = Airspace::Type::PROHIBITED; break;
				case 'R': thisCategory = Airspace::Type::RESTRICTED; break;					
				default: break;
				}
			} else {
				if (shortCategory == "TMA") thisCategory = Airspace::Type::TMA; //Terminal control areas
				else if (shortCategory == "CTR") thisCategory = Airspace::Type::CTR; //Control zones
				else if (shortCategory == "RMZ") thisCategory = Airspace::Type::RMZ; //Radio mandatory zones
				else if (shortCategory == "TMZ") thisCategory = Airspace::Type::TMZ; //Transponder mandatory zones
				else if (shortCategory == "TRA") thisCategory = Airspace::Type::RESTRICTED; //Temporary reserved airspaces
				else if (shortCategory == "MTMA") thisCategory = Airspace::Type::TMA; // Military terminal control areas
				else if (shortCategory == "MCTR") thisCategory = Airspace::Type::CTR; // Military control zones
				else if (shortCategory == "MATZ") thisCategory = Airspace::Type::CTR; // Military aerodrome traffic zones
				else if (shortCategory == "MTRA") thisCategory = Airspace::Type::RESTRICTED; // Military temporary reserved areas
				else if (shortCategory == "MTA") thisCategory = Airspace::Type::DANGER;  // Military training areas
				//else if (shortCategory == "CTA") thisCategory = Airspace::Type::UNDEFINED; //Control areas: will be converted to normal classes
			}
		}
	}
	if (thisCategory == Airspace::Type::UNDEFINED) {
		if (categoryName == "Gliding areas") thisCategory = Airspace::Type::GLIDING;
		else if (categoryName == "Hang gliding and para gliding areas") thisCategory = Airspace::Type::GLIDING;
		else if (categoryName == "Parachute jumping areas") thisCategory = Airspace::Type::DANGER;
	}
	if (thisCategory == Airspace::Type::UNDEFINED) thisCategory = upperCategory;
	folderCategory = thisCategory;

	// Visit the folder elements
	try {
		for (boost::property_tree::ptree::value_type const& element : folder) {
			if (element.first == "Placemark") ProcessPlacemark(element.second); // To find a Placemark shoud be more frequent here
			else if (element.first == "Folder") ProcessFolder(element.second, thisCategory);
		}
	}
	catch (...) {
		AirspaceConverter::LogMessage("ERROR: Exception while parsing Folder tag.", true);
		folderCategory = upperCategory;
		return false;
	}

	folderCategory = upperCategory;
	return true;
}

bool KML::ProcessPolygon(const boost::property_tree::ptree& polygon, Airspace& airspace, bool& isExtruded, Altitude& altitude) {
	// Verify if extrude and altitudeMode tags are present
	isExtruded = (polygon.count("extrude") == 1);
	bool isAGL = (polygon.count("altitudeMode") == 1);

	try {
		// First try to get the LinearRing
		boost::property_tree::ptree linearRing = polygon.get_child("outerBoundaryIs").get_child("LinearRing");

		// Get extrude and altitudeMode contents
		if (isExtruded && polygon.get<int>("extrude") != 1) isExtruded = false;
		if (isAGL && polygon.get<std::string>("altitudeMode") != "relativeToGround") isAGL = false;

		// Then get the coordinates
		std::string str = linearRing.get<std::string>("coordinates");
		if (str.empty()) return false;

		Airspace airsp;
		assert(airsp.GetNumberOfPoints() == 0);

		double avgAltitude = 0;
		unsigned long numOfPoints = 0;
		double lat = Geometry::LatLon::UNDEF_LAT, lon = Geometry::LatLon::UNDEF_LON;
		double alt = -8000;
		boost::char_separator<char> sep(", \n");
		boost::tokenizer<boost::char_separator<char> > tokens(str, sep);
		bool error(false);
		bool allPointsAtSameAlt = true;
		int expected = 0; // 0: longitude, 1: latitude, 2:altitude
		try {
			for (const std::string& c : tokens) {
				const double value = std::stod(c);
				switch (expected) {
					case 0: // longitude
						if (Geometry::LatLon::IsValidLon(value)) lon = value;
						else error = true;
						expected = 1;
						break;
					case 1: // latitude
						if (Geometry::LatLon::IsValidLat(value)) lat = value;
						else error = true;
						expected = 2;
						break;
					case 2: // altitude
						if (allPointsAtSameAlt) {
							if (alt != -8000) {
								if (value != alt) allPointsAtSameAlt = false;
							} else alt = value;
						}
						airsp.AddSinglePointOnly(lat, lon);
						numOfPoints++;
						avgAltitude += value;
						expected = 0;
						break;
					default:
						error = true;
				}
				if (error) break;
			}
		}
		catch (...) {
			error = true;
		}

		// If all OK perform additional checks
		if (!error && expected == 0) {
			// Calculate the average altitude found in this polygon
			if (allPointsAtSameAlt) avgAltitude = alt;
			else avgAltitude /= numOfPoints;

			// Ensure that the polygon is closed (it should be already)...
			airsp.ClosePoints();

			// Or the points are all at the same height or verify that all points are unique
			if (allPointsAtSameAlt || airsp.ArePointsValid()) {
				airspace.CutPointsFrom(airsp);
				altitude.SetAltMt(avgAltitude, !isAGL);
				return true;
			}
		}
	} catch(...) {}
	return false;
}

bool KML::ProcessPlacemark(const boost::property_tree::ptree& placemark) {
	// Check if it is a multi geometry
	bool isMultiGeometry(placemark.count("MultiGeometry") == 1);

	// If not must be a single polygon
	if(!isMultiGeometry && placemark.count("Polygon") != 1) return false;

	try {
		// Initialize airspace category from the folder
		Airspace::Type category = (Airspace::Type)folderCategory;

		// Build the new airspace
		Airspace airspace(category);

		// Get and set the name
		std::string str = placemark.get<std::string>("name");
		airspace.SetName(str);

		boost::property_tree::ptree schemaData = placemark.get_child("ExtendedData").get_child("SchemaData");

		bool basePresent(false), topPresent(false);
		for (boost::property_tree::ptree::value_type const& simpleData : schemaData) {
			if (simpleData.first != "SimpleData") continue;
			str = simpleData.second.get_child("<xmlattr>").get<std::string>("name");
			if (str == "Upper_Limit" || str == "Top") topPresent = AirspaceConverter::ParseAltitude(simpleData.second.data(), true, airspace);
			else if (str == "Lower_Limit" || str == "Base") basePresent = AirspaceConverter::ParseAltitude(simpleData.second.data(), false, airspace);
			else if (str == "NAM" || str == "name") {
				if (!simpleData.second.data().empty()) airspace.SetName(simpleData.second.data());
			}
			else if (str == "Category") {
				if (simpleData.second.data() == "Class A") category = Airspace::Type::CLASSA;
				else if (simpleData.second.data() == "Class B") category = Airspace::Type::CLASSB;
				else if (simpleData.second.data() == "Class C") category = Airspace::Type::CLASSC;
				else if (simpleData.second.data() == "Class D") category = Airspace::Type::CLASSD;
				else if (simpleData.second.data() == "Class E") category = Airspace::Type::CLASSE;
				else if (simpleData.second.data() == "Class F") category = Airspace::Type::CLASSF;
				else if (simpleData.second.data() == "Class G") category = Airspace::Type::CLASSG;
				else if (simpleData.second.data() == "Danger") category = Airspace::Type::DANGER;
				else if (simpleData.second.data() == "Prohibited") category = Airspace::Type::PROHIBITED;
				else if (simpleData.second.data() == "Restricted") category = Airspace::Type::RESTRICTED;
				else if (simpleData.second.data() == "CTR") category = Airspace::Type::CTR;
				else if (simpleData.second.data() == "TMA") category = Airspace::Type::TMA;
				else if (simpleData.second.data() == "TMZ") category = Airspace::Type::TMZ;
				else if (simpleData.second.data() == "RMZ") category = Airspace::Type::RMZ;
				else if (simpleData.second.data() == "FIR") category = Airspace::Type::FIR;
				else if (simpleData.second.data() == "UIR") category = Airspace::Type::UIR;
				else if (simpleData.second.data() == "OTH") category = Airspace::Type::OTH;
				else if (simpleData.second.data() == "Gliding area") category = Airspace::Type::GLIDING;
				else if (simpleData.second.data() == "No glider") category = Airspace::Type::NOGLIDER;
				else if (simpleData.second.data() == "Wave window") category = Airspace::Type::WAVE;
				else if (simpleData.second.data() == "Unknown") category = Airspace::Type::UNKNOWN;
				else AirspaceConverter::LogMessage("ERROR: Unable to parse airspace category in the label: " + simpleData.second.data(), true);
			}
		}

		// If found a category from the label use it
		if (category != airspace.GetType()) airspace.SetType(category);

		// Try to find the airspace class (for CTA, TMA and CTR) or the category from the name
		airspace.GuessClassFromName();

		// If still invalid category skip it
		if (airspace.GetType() == Airspace::Type::UNDEFINED) return false;

		bool pointsFound(false);

		// If we expect a multigeometry...
		if(isMultiGeometry) {

			// If both altitudes were already found in the "header", then it's easier
			if(basePresent && topPresent) {
				// Iterate trough all the polygons of multigeometry just to find the points
				for (boost::property_tree::ptree::value_type const& polygon : placemark.get_child("MultiGeometry")) {
					if (polygon.first != "Polygon") continue;
					Altitude alt;
					bool isExtruded(true);
					pointsFound = ProcessPolygon(polygon.second, airspace, isExtruded, alt);
					if (isExtruded) return false; // Polygon of multigeometry must not be extruded

					// if found it, no need to continue iterating over all the other polygons...
					if(pointsFound) break;
				}
			}

			// If the altitudes were not found look for them from the polygons...
			else {
				Altitude top(airspace.GetTopAltitude()), base(airspace.GetBaseAltitude());
				if (!basePresent) base.SetAltMt(1000000);
				if (!topPresent) top.SetAltMt(-8000);
				bool baseFound(basePresent), topFound(topPresent);

				// Iterate trough all the polygons of multigeometry
				for (boost::property_tree::ptree::value_type const& polygon : placemark.get_child("MultiGeometry")) {
					if (polygon.first != "Polygon") continue;
					Altitude foundAlt;
					bool isExtruded(true);

					if(ProcessPolygon(polygon.second, airspace, isExtruded, foundAlt)) {
						// There should be no extruded polygons in a multigeometry
						if (isExtruded) return false;

						if (!topPresent && foundAlt > top) {
							top = foundAlt;
							topFound = true;
						}
						if(!basePresent && foundAlt < base) {
							base = foundAlt;
							baseFound = true;
						}
					}
				}
				if(baseFound && topFound) pointsFound = true;

				// If points found verify the altitudes found
				if (pointsFound) {
					if (!basePresent) {
						if (baseFound) airspace.SetBaseAltitude(base);
						else {
							AirspaceConverter::LogMessage("Warning: skipping MultiGeometry with invalid base altitude: " + airspace.GetName(), false);
							return false;
						}
					}
					if (!topPresent) {
						if (topFound) airspace.SetTopAltitude(top);
						else {
							AirspaceConverter::LogMessage("Warning: skipping MultiGeometry with invalid top altitude: " + airspace.GetName(), false);
							return false;
						}
					}
				}
			}
		}

		// Otherwise it should be a single extruded polygon
		else {
			bool isExtruded(false);
			Altitude foundAlt;
			pointsFound = ProcessPolygon(placemark.get_child("Polygon"), airspace, isExtruded, foundAlt);
			if (pointsFound) {
				if (isExtruded) {
					if (!basePresent) { // Extruded means GND base
						Altitude gnd;
						gnd.SetGND();
						airspace.SetBaseAltitude(gnd);
					}
				} else return false; // The single polygon should be always extruded
				if (!topPresent) airspace.SetTopAltitude(foundAlt);
			}
		}

		if (pointsFound) {
			// Check if the altitudes make sense
			if (airspace.GetBaseAltitude() < airspace.GetTopAltitude()) {
				airspaces.insert(std::pair<int, Airspace>(airspace.GetType(), std::move(airspace)));
				return true;
			} else AirspaceConverter::LogMessage("Warning: skipping Placemark with invalid altitudes: " + airspace.GetName(), false);
		}

	} catch (...) {}
	return false;
}

bool KML::ReadKML(const std::string& filename) {
	std::ifstream input(filename);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogMessage("ERROR: Unable to open KML file: " + filename, true);
		return false;
	}
	AirspaceConverter::LogMessage("Reading KML file: " + filename, false);
	boost::property_tree::ptree root;
	boost::property_tree::read_xml(input, root);
	input.close();
	try {
		boost::property_tree::ptree doc = root.get_child("kml").get_child("Document");
		for (boost::property_tree::ptree::value_type const& element : doc) {
			if (element.first == "Folder") ProcessFolder(element.second, Airspace::Type::UNDEFINED);
			else if (element.first == "Placemark") ProcessPlacemark(element.second);
		}
	} catch (...) {
		AirspaceConverter::LogMessage("ERROR: Exception while parsing basic elements of KML file.", true);
		return false;
	}
	return true;
}
