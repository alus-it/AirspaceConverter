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

//#include <boost/property_tree/xml_parser.hpp>
//#include <boost/property_tree/ptree.hpp>


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

void KML::WriteHeader(const bool airspace, const bool waypoints) {
	assert(airspace || waypoints);
	outputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<!--\n";
	for(const std::string& line: AirspaceConverter::disclaimer) outputFile << line << "\n";
	outputFile << "-->\n"
		<< "<kml xmlns = \"http://www.opengis.net/kml/2.2\">\n"
		<< "<Document>\n"
		<< "<open>true</open>\n";
		if (airspace) for (int t = Airspace::CLASSA; t <= Airspace::UNDEFINED; t++) {
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
		if (waypoints) for (int t = Waypoint::normal; t < Waypoint::numOfWaypointTypes; t++) {
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

void KML::OpenPlacemark(const Waypoint& waypoint) {
	const bool isAirfield = waypoint.IsAirfield();
	const int altMt = waypoint.GetAltitude();
	const int altFt = (int)round(altMt / Altitude::FEET2METER);
	outputFile << "<Placemark>\n"
		<< "<name>" << waypoint.GetName() << "</name>\n"
		<< "<styleUrl>#Style" << waypoint.GetTypeName() << "</styleUrl>\n";
	outputFile << "<visibility>" << (isAirfield ? 1 : 0) << "</visibility>\n"
		<< "<ExtendedData>\n"
		<< "<SchemaData>\n"
		<< "<SimpleData name=\"Name\">" << waypoint.GetName() << "</SimpleData>\n"
		<< "<SimpleData name=\"Type\">" << waypoint.GetTypeName() << "</SimpleData>\n"
		<< "<SimpleData name=\"Code\">" << waypoint.GetCode() << "</SimpleData>\n"
		<< "<SimpleData name=\"Country\">" << waypoint.GetCountry() << "</SimpleData>\n"
		<< "<SimpleData name=\"Altitude\">" << altMt << " m - " << altFt << " ft" << "</SimpleData>\n";
	if(isAirfield) {
		const Airfield& airfield = (const Airfield&)waypoint;
		outputFile << "<SimpleData name=\"Runway direction\">" << (airfield.GetRunwayDir() != -1 ? std::to_string(airfield.GetRunwayDir()) + " deg" : "UNKNOWN") << "</SimpleData>\n"
		<< "<SimpleData name=\"Runway length\">" << (airfield.GetRunwayLength() != -1 ? std::to_string(airfield.GetRunwayLength()) + " m" : "UNKNOWN") << "</SimpleData>\n"
		<< "<SimpleData name=\"Radio frequency\">" << (airfield.GetRadioFrequency().empty() ? "UNKNOWN" : airfield.GetRadioFrequency() + " MHz") << "</SimpleData>\n";
	}
	outputFile << "<SimpleData name=\"Description\">" << waypoint.GetDescription() << "</SimpleData>\n"
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
				const Waypoint& w = it->second;

				// Open placemark
				OpenPlacemark(w);

				// Flag to remember if the runway perimeter has been drawn
				bool airfieldDrawn = false;

				int dir = -1;
				
				// If it is an airfield draw an estimation of the runway perimeter
				if (isAirfield) {
					
					// Get the airfield
					const Airfield& a = (const Airfield&)w;
					
					// Get its rinway length and direction
					const int leng = a.GetRunwayLength();
					dir = a.GetRunwayDir();

					// If they are valid...
					if (leng > 0 && dir >= 0) {

						// Calculate the runway perimeter
						std::vector<Geometry::LatLon> airfieldPerimeter;
						if (Geometry::CalcAirfieldPolygon(a.GetLatitude(), a.GetLongitude(), leng, dir, airfieldPerimeter)) {
							
							// Open a multigeometry with a polygon clamped onto the ground
							outputFile << "<MultiGeometry>\n"
								<< "<Polygon>\n"
								//<< "<altitudeMode>clampToGround</altitudeMode>\n" //this should be the default
								<< "<outerBoundaryIs>\n"
								<< "<LinearRing>\n"
								<< "<coordinates>\n";

							// Add the four points
							for (const Geometry::LatLon& p : airfieldPerimeter)
								outputFile << p.Lon() << "," << p.Lat() << "," << a.GetAltitude() << "\n";
							
							// Close the perimeter re-adding the first point 
							outputFile << airfieldPerimeter.front().Lon() << "," << airfieldPerimeter.front().Lat() << "," << a.GetAltitude() << "\n";

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
					<< "<coordinates>" << w.GetLongitude() << "," << w.GetLatitude() << "," << w.GetAltitude() << "</coordinates>\n"
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

				//TODO: if we have a terrain map for that area should be anyway better to get the AGL altitudes converted to AMSL
				if (a.IsGNDbased()) WriteBaseOrTop(a, a.GetTopAltitude(), true); // then that's easy!
				else { // otherwise we have to abuse KML which is not properly done to draw middle air aispaces
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
	AirspaceConverter::LogMessage("Compressing into KMZ: doc.kmz", false);

	// To avoid problems it is better to delete the KMZ file if already existing, user has already been warned
	if (boost::filesystem::exists(filename)) std::remove(filename.c_str()); // Delete KMZ file

	// Open the ZIP file
	int error = 0;
	zip *archive = zip_open(filename.c_str(), ZIP_CREATE, &error);
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

bool KML::Read(const std::string& filename) {
	//TODO: to be implemented ...
	return false;
}
