//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "KML.hpp"
#include "Airspace.hpp"
#include "AirspaceConverter.hpp"
#include "Waypoint.hpp"
#include "Airfield.hpp"
#include "Geometry.hpp"
#include <zip.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/tokenizer.hpp>
#include <cmath>

const std::string KML::colors[Airspace::Type::UNDEFINED] = {
	"9900ff", //CLASSA
	"cc0000", //CLASSB
	"cc3399", //CLASSC
	"ff9900", //CLASSD
	"339900", //CLASSE
	"3399ff", //CLASSF
	"ff99ff", //CLASSG
	"f42e0e", //D
	"0000ff", //P
	"ff0080", //R
	"1947ff", //CTR
	"f8d1ab", //TMZ
	"857037", //RMZ
	"00e2e2", //GLIDING
	"d4d4d4", //NOGLIDER
	"a6789f", //WAVE
	"ebdf1a", //NOTAM
	"d4d4d4", //OTHER
	"ffdd01", //TMA
	"82f2d6", //FIR
	"13a8b9", //UIR
	"ebbd87", //OTH
	"ef1679", //AWY
	"107b0d", //MATZ
	"5de959", //MTMA
	"e9541c", //MTRA
	"c38711", //TFR
	"dacb0e", //ADA
	"fc7c0b", //ADIZ
	"0b48fc", //CTA
	"0bb2fc", //DFIR
	"03abdb", //TIZ
	"03dbc6", //TIA
	"bc75f4", //SRZ
	"2f30df", //ATZ
	"5bb5db", //FISA
	"ba715e", //MBZ
	"f1b51b", //ASR
	"caf11b", //COMP
	"92775e", //TRZ
	"ac60e5", //VFRR
	"844857", //RTZ
	"ef3f2b", //PARA
	"cd2175", //LFZ
	"bf91d7", //CFZ
	"10861e", //MOA
	"64974f", //MTA
	"7248e2", //TSA
	"8c64ba", //TRA
	"d4d4d4"  //UNKNOWN
};

const std::string KML::airfieldColors[] = {
	"14f064",	//AirfieldGrass
	"143c64",	//Outlanding
	"fafafa",	//GliderSite
	"6e6e6e",	//AirfieldSolid
};

const std::string KML::waypointIcons[Waypoint::WaypointType::numOfWaypointTypes] = {
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

const std::string KML::iconsPath(DetectIconsPath());

const std::string KML::DetectIconsPath() {
	std::string 
#ifdef __APPLE__
	// Default installed CLI macOS location
	path("/usr/local/share/airspaceconverter/icons/");
	if (boost::filesystem::exists(path)) return path;

	// Default installed GUI macOS location
	path = boost::filesystem::path(boost::filesystem::path(AirspaceConverter::basePath) / boost::filesystem::path("../Resources/icons/")).string();
	if (boost::filesystem::exists(path)) return path;
#elif __linux__
	path("/usr/share/airspaceconverter/icons/"); // Default installed Linux location
	if (boost::filesystem::exists(path)) return path;
#endif
	path = boost::filesystem::path(boost::filesystem::path(AirspaceConverter::basePath) / boost::filesystem::path("icons/")).string();
	if (boost::filesystem::exists(path)) return path;
	return "./icons/";
}

KML::KML(std::multimap<int, Airspace>& airspacesMap, std::multimap<int, Waypoint*>& waypointsMap):
		airspaces(airspacesMap),
		waypoints(waypointsMap),
		allAGLaltitudesCovered(true),
		processLineString(false),
		folderCategory(Airspace::Type::UNDEFINED) {
}

std::string KML::PrepareTagText(const std::string& text) {
	std::string preparedText;
	preparedText.reserve((size_t)(text.size() * 1.1));
	for (size_t pos = 0; pos != text.size(); ++pos) {
		switch (text[pos]) {
		case '&':
			preparedText.append("&amp;");
			break;
		case '\"':
			preparedText.append("&quot;");
			break;
		case '\'':
			preparedText.append("&apos;");
			break;
		case '<':
			preparedText.append("&lt;");
			break;
		case '>':
			preparedText.append("&gt;");
			break;
		default:
			preparedText.append(&text[pos], 1);
			break;
		}
	}
	return preparedText;
}

void KML::WriteHeader(const bool airspacePresent, const bool waypointsPresent) {
	assert(airspacePresent || waypointsPresent);
	outputFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<!--\n";
	for(const std::string& line: AirspaceConverter::disclaimer) outputFile << line << "\n";
	outputFile << "\n" << AirspaceConverter::GetCreationDateString() << " -->\n"
		<< "<kml xmlns = \"http://www.opengis.net/kml/2.2\">\n"
		<< "<Document>\n"
		<< "<open>true</open>\n";
		if (airspacePresent) {
			for (int t = Airspace::CLASSA; t < Airspace::UNDEFINED; t++) {
				outputFile << "<Style id = \"Style" << Airspace::CategoryName((Airspace::Type)t) << "\">\n"
					<< "<LineStyle>\n"
					<< "<color>50" << colors[t] << "</color>\n"
					<< "<width>1.5</width>\n"
					<< "</LineStyle>\n"
					<< "<PolyStyle>\n"
					<< "<color>55" << colors[t] << "</color>\n"
					<< "</PolyStyle>\n"
					<< "</Style>\n";
			}
			outputFile << "<Schema name=\"\" id=\"AirspaceId\">\n"
				<< "<SimpleField type=\"string\" name=\"Name\">\n"
				<< "<displayName><![CDATA[<b>Name:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Category\">\n"
				<< "<displayName><![CDATA[<b>Category:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Top\">\n"
				<< "<displayName><![CDATA[<b>Ceiling:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Base\">\n"
				<< "<displayName><![CDATA[<b>Floor:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Radio\">\n"
				<< "<displayName><![CDATA[<b>Radio frequency (MHz):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Xpndr\">\n"
				<< "<displayName><![CDATA[<b>Transponder code:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"double\" name=\"Area\">\n"
				<< "<displayName><![CDATA[<b>Area (Km<sup>2</sup>):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"double\" name=\"Perimeter\">\n"
				<< "<displayName><![CDATA[<b>Perimeter (Km):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "</Schema>\n";
		}
		if (waypointsPresent) {
			for (int t = Waypoint::normal; t < Waypoint::numOfWaypointTypes; t++) {
				outputFile << "<Style id = \"Style" << Waypoint::TypeName(t) << "\">\n";

				// Put the icon only if there is the right icon PNG file availble
				if (boost::filesystem::exists(iconsPath + waypointIcons[t])) {
					outputFile << "<IconStyle>\n"
						<< "<Icon>\n"
						<< "<href>icons/" << waypointIcons[t] << "</href>\n"
						<< "</Icon>\n"
						<< "</IconStyle>\n";
				}
				if (Waypoint::IsTypeAirfield(t))
					outputFile << "<LineStyle>\n"
						<< "<color>4b" << airfieldColors[t-2] << "</color>\n"
						<< "<width>1.5</width>\n"
						<< "</LineStyle>\n"
						<< "<PolyStyle>\n"
						<< "<color>37" << airfieldColors[t-2] << "</color>\n"
						<< "</PolyStyle>\n";
				outputFile << "</Style>\n";
			}
			outputFile << "<Schema name=\"\" id=\"WaypointId\">\n"
				<< "<SimpleField type=\"string\" name=\"Name\">\n"
				<< "<displayName><![CDATA[<b>Name:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Type\">\n"
				<< "<displayName><![CDATA[<b>Type:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Code\">\n"
				<< "<displayName><![CDATA[<b>Code:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Country\">\n"
				<< "<displayName><![CDATA[<b>Country:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"int\" name=\"AltMt\">\n"
				<< "<displayName><![CDATA[<b>Altitude (m):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"int\" name=\"AltFt\">\n"
				<< "<displayName><![CDATA[<b>Altitude (ft):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"int\" name=\"RwyDir\">\n"
				<< "<displayName><![CDATA[<b>Runway direction (deg):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"int\" name=\"RwyLen\">\n"
				<< "<displayName><![CDATA[<b>Runway length (m):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"double\" name=\"Radio\">\n"
				<< "<displayName><![CDATA[<b>Radio frequency (MHz):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"string\" name=\"Desc\">\n"
				<< "<displayName><![CDATA[<b>Description:</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"double\" name=\"VOR\">\n"
				<< "<displayName><![CDATA[<b>VOR frequency (MHz):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "<SimpleField type=\"double\" name=\"NDB\">\n"
				<< "<displayName><![CDATA[<b>NDB frequency (kHz):</b>]]></displayName>\n"
				<< "</SimpleField>\n"
				<< "</Schema>\n";
		}
}

void KML::OpenPlacemark(const Airspace& airspace) {
	const std::string name(PrepareTagText(airspace.GetName()));
	std::string longName(name);
	if (airspace.GetClass() != Airspace::UNDEFINED && (airspace.GetType() == Airspace::CTR || airspace.GetType() == Airspace::TMA)) longName.append(" - " + Airspace::CategoryName(airspace.GetClass()));
	double area(0), perimeter(0);
	airspace.CalculateSurface(area, perimeter);
	outputFile << "<Placemark>\n"
		<< "<name>" << name << "</name>\n"
		<< "<styleUrl>#Style" << airspace.GetCategoryName() << "</styleUrl>\n"
		<< "<visibility>" << (airspace.IsVisibleByDefault() ? 1 : 0) << "</visibility>\n"
		<< "<ExtendedData>\n"
		<< "<SchemaData schemaUrl=\"#AirspaceId\">\n"
		<< "<SimpleData name=\"Name\">" << longName << "</SimpleData>\n"
		<< "<SimpleData name=\"Category\">" <<  airspace.GetLongCategoryName() << "</SimpleData>\n"
		<< "<SimpleData name=\"Top\">" << airspace.GetTopAltitude().ToString() << "</SimpleData>\n"
		<< "<SimpleData name=\"Base\">" << airspace.GetBaseAltitude().ToString() << "</SimpleData>\n";
	outputFile << std::fixed << std::setprecision(3);
	for (size_t i=0; i<airspace.GetNumberOfRadioFrequencies(); i++) {
		const std::pair<int, std::string>& f = airspace.GetRadioFrequencyAt(i);
		outputFile << "<SimpleData name=\"Radio\">";
		if (!f.second.empty()) outputFile << f.second << ": ";
		outputFile << AirspaceConverter::FrequencyMHz(f.first) << "</SimpleData>\n";
	}
	if (airspace.HasTransponderCode()) outputFile << "<SimpleData name=\"Xpndr\">" << airspace.GetTransponderCode() << "</SimpleData>\n";
	outputFile << std::setprecision(1) << "<SimpleData name=\"Area\">" << area << "</SimpleData>\n"
		<< "<SimpleData name=\"Perimeter\">" << perimeter << "</SimpleData>\n"
		<< "</SchemaData>\n"
		<< "</ExtendedData>\n";
	outputFile.unsetf(std::ios_base::floatfield); //outputFile << std::defaultfloat; not supported by older GCC 4.9.0
}

void KML::OpenPlacemark(const Waypoint* waypoint) {
	const bool isAirfield = waypoint->IsAirfield();
	const int altMt = (int)std::round(waypoint->GetAltitude());
	const int altFt = (int)std::round(altMt / Altitude::FEET2METER);
	outputFile << "<Placemark>\n"
		<< "<name>" << PrepareTagText(waypoint->GetName()) << "</name>\n"
		<< "<styleUrl>#Style" << waypoint->GetTypeName() << "</styleUrl>\n";
	outputFile << "<visibility>" << (isAirfield ? 1 : 0) << "</visibility>\n"
		<< "<ExtendedData>\n"
		<< "<SchemaData schemaUrl=\"#WaypointId\">\n"
		<< "<SimpleData name=\"Name\">" << PrepareTagText(waypoint->GetName()) << "</SimpleData>\n"
		<< "<SimpleData name=\"Type\">" << waypoint->GetTypeName() << "</SimpleData>\n"
		<< "<SimpleData name=\"Code\">" << PrepareTagText(waypoint->GetCode()) << "</SimpleData>\n"
		<< "<SimpleData name=\"Country\">" << waypoint->GetCountry() << "</SimpleData>\n"
		<< "<SimpleData name=\"AltMt\">" << altMt << "</SimpleData>\n"
		<< "<SimpleData name=\"AltFt\">" << altFt << "</SimpleData>\n";
	outputFile << std::fixed << std::setprecision(3);
	if(isAirfield) {
		const Airfield* airfield = (const Airfield*)waypoint;
		if (airfield->HasRunwayDir()) outputFile << "<SimpleData name=\"RwyDir\">" << airfield->GetRunwayDir() << "</SimpleData>\n";
		if (airfield->HasRunwayLength()) outputFile << "<SimpleData name=\"RwyLen\">" << airfield->GetRunwayLength() << "</SimpleData>\n";
		if (airfield->HasRadioFrequency()) outputFile << "<SimpleData name=\"Radio\">" << AirspaceConverter::FrequencyMHz(airfield->GetRadioFrequency()) << "</SimpleData>\n";
		if (airfield->HasOtherFrequency()) outputFile << "<SimpleData name=\"Radio\">" << AirspaceConverter::FrequencyMHz(airfield->GetOtherFrequency()) << "</SimpleData>\n";
	}
	if (waypoint->HasOtherFrequency()) {
		if (waypoint->GetType() == Waypoint::WaypointType::VOR)
			outputFile << "<SimpleData name=\"VOR\">" << AirspaceConverter::FrequencyMHz(waypoint->GetOtherFrequency()) << "</SimpleData>\n";
		else if (waypoint->GetType() == Waypoint::WaypointType::NDB)
			outputFile << "<SimpleData name=\"NDB\">" << std::setprecision(1) << AirspaceConverter::FrequencykHz(waypoint->GetOtherFrequency()) << "</SimpleData>\n";
	}
	outputFile.unsetf(std::ios_base::floatfield); //outputFile << std::defaultfloat; not supported by older GCC 4.9.0
	outputFile << "<SimpleData name=\"Desc\">" << PrepareTagText(waypoint->GetDescription()) << "</SimpleData>\n"
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
	outputFile << std::setprecision(6);
	double altitude = alt.GetAltMt();
	for (const Geometry::LatLon& p : airspace.GetPoints()) outputFile << p.Lon() << "," << p.Lat() << "," << altitude << "\n";
	ClosePolygon();
}

void KML::WriteBaseOrTop(const Airspace& airspace, const std::vector<double>& altitudesAmsl, const bool extrudeToGround /*= false*/) {
	OpenPolygon(extrudeToGround, true);
	assert(airspace.GetNumberOfPoints() == altitudesAmsl.size());
	outputFile << std::setprecision(6);
	for (size_t i = 0; i < altitudesAmsl.size(); i++) {
		const Geometry::LatLon p = airspace.GetPointAt(i);
		outputFile << p.Lon() << "," << p.Lat() << "," << altitudesAmsl.at(i) << "\n";
	}
	ClosePolygon();
}

void KML::WriteSideWalls(const Airspace& airspace) {
	assert(airspace.GetTopAltitude().IsAMSL() == airspace.GetBaseAltitude().IsAMSL());
	const double top = airspace.GetTopAltitude().GetAltMt();
	const double base = airspace.GetBaseAltitude().GetAltMt();
	double lon1, lat1, lon2, lat2;

	// Build walls from first, point to point, until the last one
	for (size_t i = 0; i < airspace.GetNumberOfPoints() - 1; i++) {
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
	assert(airspace.GetNumberOfPoints() == altitudesAmsl.size());
	const bool isBase = airspace.GetBaseAltitude().IsAGL();
	double top1, base1, top2, base2, lon1, lat1, lon2, lat2;

	// Build walls from first, point to point, until the last one
	for (size_t i = 0; i < airspace.GetNumberOfPoints() - 1; i++) {
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
	if((!airspacesPresent && !waypointsPresent) || filename.empty()) {
		AirspaceConverter::LogMessage("KML output: no airspace and no waypoints, nothing to write");
		return false;
	}
	
	// The file must be a KMZ
	if (!boost::iequals(boost::filesystem::path(filename).extension().string(), ".kmz")) {
		AirspaceConverter::LogError("Expected KMZ extension but found: " + boost::filesystem::path(filename).extension().string());
		return false;
	}

	// Prepare pathname to the KML doc.kml; KMZ files should have the KML file name named as "doc.kml"
	const std::string fileKML(boost::filesystem::path(boost::filesystem::path(filename).parent_path() / boost::filesystem::path("doc.kml")).string());

	// Make sure the file is not already open
	if (outputFile.is_open()) outputFile.close();
	
	// Open the output file
	outputFile.open(fileKML, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!outputFile.is_open() || outputFile.bad()) {
		AirspaceConverter::LogError("Unable to open output file: " + filename);
		return false;
	}
	AirspaceConverter::LogMessage("Writing output file: " + fileKML);

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
		for (int t = Waypoint::unknown; t < Waypoint::numOfWaypointTypes; t++) {

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
					if (leng > 0 && dir > 0) {

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
							outputFile << std::setprecision(6);
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
					<< "<coordinates>" << std::setprecision(6) << w->GetLongitude() << "," << w->GetLatitude() << "," << (int)std::round(w->GetAltitude()) << "</coordinates>\n"
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
		for (int t = Airspace::CLASSA; t < Airspace::UNDEFINED; t++) {

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

				if ((a.IsMSLbased() && a.IsAMSLtopped()) || (a.IsGNDbased() && a.IsAMSLtopped())) WriteBaseOrTop(a, a.GetTopAltitude(), true); // base on the sea or on graund and AMSL top: that's easy!
				else if ((a.IsMSLbased() && a.IsAGLtopped()) || (a.IsGNDbased() && a.IsAGLtopped())) { // in this case it's more complicated

					const double altitudeAGLmt = a.GetTopAltitude().GetAltMt();

					// Try to get terrein altitude then add the AGL altitude to get AMSL altitude
					std::vector<double> amslAltitudesMt;
					for (const Geometry::LatLon& p : a.GetPoints()) {
						double terrainHeightMt = AirspaceConverter::GetDefaultTerrainAlt();
						allAGLaltitudesCovered = AirspaceConverter::GetTerrainAltitudeMt(p.Lat(), p.Lon(), terrainHeightMt) && allAGLaltitudesCovered;
						amslAltitudesMt.push_back(terrainHeightMt + altitudeAGLmt);
					}

					// Write the top points reobtained as AMSL
					WriteBaseOrTop(a, amslAltitudesMt, true);
	
				} else { // otherwise we have to misuse even more KML which is not properly done to draw middle air airspaces
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
						const double altitudeAGLmt = (a.GetBaseAltitude().IsAGL() ? a.GetBaseAltitude() : a.GetTopAltitude()).GetAltMt();

						// Try to get terrein altitude then add the AGL altitude to get AMSL altitude
						std::vector<double> amslAltitudesMt;
						for (const Geometry::LatLon& p : a.GetPoints()) {
							double terrainHeightMt = AirspaceConverter::GetDefaultTerrainAlt();
							allAGLaltitudesCovered = AirspaceConverter::GetTerrainAltitudeMt(p.Lat(), p.Lon(), terrainHeightMt) && allAGLaltitudesCovered;
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
	AirspaceConverter::LogMessage("Compressing into KMZ: " + filename);

	// To avoid problems it is better to delete the KMZ file if already existing, user has already been warned
	if (boost::filesystem::exists(filename)) std::remove(filename.c_str()); // Delete KMZ file

	// Open the ZIP file
	int error = 0;
	zip* archive = zip_open(filename.c_str(), ZIP_CREATE, &error);
	if (error) {
		AirspaceConverter::LogError("Could not open or create archive: " + filename);
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
		AirspaceConverter::LogError("Failed to create zip source buffer to read: " + fileKML);
		return false;
	}

	// Add the buffer as KLM file in the ZIP
#ifdef ZIP_FL_OVERWRITE
	int index = (int)zip_file_add(archive, "doc.kml", source, ZIP_FL_OVERWRITE);
#else
	int index = (int)zip_add(archive, "doc.kml", source);
#endif
	if (index < 0) { // "failed to add file to archive. " << zip_strerror(archive)
#ifdef ZIP_FL_OVERWRITE
		zip_discard(archive);
#else
		zip_close(archive);
#endif
		zip_source_free(source); // The sorce buffer have to be freed in this case
		AirspaceConverter::LogError("While compressing, failed to add: doc.kml");
		return false;
	}

	// If it is necessary to add also the icons
	if (!waypoints.empty()) {
		for (int i = Waypoint::unknown; i < Waypoint::numOfWaypointTypes; i++) {
			// Get the icon PNG filename and prepare the path in the ZIP and the path from current dir
			const std::string iconPath = iconsPath + waypointIcons[i];

			// Check if we can get that PNG file
			if (!boost::filesystem::exists(iconPath)) {
				AirspaceConverter::LogWarning("Skipping non existant icon PNG file: " + iconPath);
				continue;
			}

			// Create source buffer from KML file
			source = zip_source_file(archive, iconPath.c_str(), 0, 0);
			if (source == nullptr) { // "failed to create source buffer. " << zip_strerror(archive)
#ifdef ZIP_FL_OVERWRITE
				zip_discard(archive);
#else
				zip_close(archive);
#endif
				AirspaceConverter::LogError("Failed to create zip source buffer to read: " + iconPath);
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
				AirspaceConverter::LogError("While compressing, failed to add: " + iconFile);
				return false;
			}
		}
	}

	// Close the zip
	if (zip_close(archive) == 0) {
		std::remove(fileKML.c_str()); // Delete KML file
		return true;
	}
	AirspaceConverter::LogError("While finalizing the archive.");
	return false;
}

bool KML::ReadKMZ(const std::string& filename) {
	// Open the ZIP file
	int error = 0;
	zip* archive = zip_open(filename.c_str(), 0, &error);
	if (error) {
		AirspaceConverter::LogError("Could not open KMZ file: " + filename);
		return false;
	}

	// Get the number of files in the ZIP
	const long nFiles = (long)zip_get_num_entries(archive, 0);
	if(nFiles < 1) {
		AirspaceConverter::LogError("KMZ file seems empty or not valid: " + filename);
		zip_close(archive);
		return false;
	}

	AirspaceConverter::LogMessage("Opened KMZ file: " + filename);

	std::string extractedKmlFile;
	struct zip_stat sb;

	// Iterate trough the contents: look for the first KML file in the root of the ZIP file
	for (long i=0; i<nFiles; i++) {
		if (zip_stat_index(archive, i, 0, &sb) != 0) {
			AirspaceConverter::LogError("while reading KMZ, unable to get details of a file in the ZIP.");
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
			AirspaceConverter::LogError("while extracting, unable to open KML file from KMZ: " + filename);
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
			AirspaceConverter::LogError("While extracting KML file, unable to write: " + extractedKmlFile);
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
				AirspaceConverter::LogError("While extracting KML file, unable read compressed data from: " + filename);
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

		AirspaceConverter::LogMessage("Extracted KML file: " + std::string(sb.name));

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
	const std::string categoryName = folder.get<std::string>("name"); // Try to guess the category from the name of folder
	Airspace::Type thisCategory = Airspace::Type::UNDEFINED;
	const std::string::size_type first = categoryName.find('(');
	if (first != std::string::npos) {
		const std::string::size_type last = categoryName.find(')');
		if (last != std::string::npos && first < last) {
			const std::string shortCategory = categoryName.substr(first+1, last-first-1);
			bool found = false;
			unsigned int cat = Airspace::Type::CLASSA;
			do {
				if (shortCategory == Airspace::CategoryName((Airspace::Type)cat)) found = true;
				else cat++;
			} while (cat < Airspace::Type::UNDEFINED && !found);
			if (found) {
				thisCategory = (Airspace::Type)cat;
				if (thisCategory == Airspace::Type::CLASSD && categoryName == "Danger areas (D)") thisCategory = Airspace::Type::D;
			}
		}
	}
	if (thisCategory == Airspace::Type::UNDEFINED) {
		if (categoryName == "Gliding areas") thisCategory = Airspace::Type::GLIDING;
		else if (categoryName == "Hang gliding and para gliding areas") thisCategory = Airspace::Type::GLIDING;
		else if (categoryName == "Parachute jumping areas") thisCategory = Airspace::Type::PARA;
	}
	if (thisCategory == Airspace::Type::UNDEFINED) thisCategory = (Airspace::Type)upperCategory;
	folderCategory = thisCategory;

	// Visit the folder elements
	try {
		for (boost::property_tree::ptree::value_type const& element : folder) {
			if (element.first == "Placemark") ProcessPlacemark(element.second); // To find a Placemark shoud be more frequent here
			else if (element.first == "Folder") ProcessFolder(element.second, thisCategory);
		}
	}
	catch (...) {
		AirspaceConverter::LogError("Exception while parsing Folder tag.");
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
		double avgAltitude = 0;
		if (ProcessCoordinates(linearRing, airspace, avgAltitude)) {
			altitude.SetAltMt(avgAltitude, !isAGL);
			return true;
		}
	} catch(...) {}
	return false;
}

bool KML::ProcessCoordinates(const boost::property_tree::ptree& parent, Airspace& airspace, double& avgAltitude) {
	try {
		std::string str = parent.get<std::string>("coordinates");
		if (str.empty()) return false;

		Airspace airsp;
		assert(airsp.GetNumberOfPoints() == 0);
		assert(avgAltitude == 0);

		bool allPointsAtSameAlt = true;
		unsigned long numOfPoints = 0;
		double lat = Geometry::LatLon::UNDEF_LAT, lon = Geometry::LatLon::UNDEF_LON;
		double alt = -8000;
		boost::char_separator<char> sep(", \n\t");
		boost::tokenizer<boost::char_separator<char> > tokens(str, sep);
		bool error(false);

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
						}
						else alt = value;
					}
					if (airsp.AddPointLatLonOnly(lat, lon)) {
						numOfPoints++;
						avgAltitude += value;
					}
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

			// Ensure that the polygon is closed (it should be already, not for LineString)...
			// ... and the points are all at the same height or verify that all points are unique
			if (airsp.ClosePoints() && (allPointsAtSameAlt || airsp.ArePointsValid())) {
				airspace.CutPointsFrom(airsp);
				return true;
			}
		}
	} catch (...) {}
	return false;
}

bool KML::ProcessPlacemark(const boost::property_tree::ptree& placemark) {
	// Check if it is a multi geometry
	bool isMultiGeometry(placemark.count("MultiGeometry") == 1);
	
	// If not check if it is a single polygon
	bool isPolygon (!isMultiGeometry && placemark.count("Polygon") == 1);

	// If not check if we want to treat LineStrings as Airspaces and if is a LineString otherwise return
	if (!isMultiGeometry && !isPolygon) {
		if (processLineString) {
			if (placemark.count("LineString") != 1) return false;
		}
		else return false;
	}

	try {
		// Initialize airspace category from the folder
		Airspace::Type category = (Airspace::Type)folderCategory;

		// Build the new airspace
		Airspace airspace(category);

		// If present get and set the name
		if (placemark.count("name") > 0) {
			airspace.SetName(placemark.get<std::string>("name"));

			// Try to find the airspace class (for CTA, TMA and CTR) or the category from the name
			airspace.GuessClassFromName();
		}

		bool basePresent(false), topPresent(false);

		if (isMultiGeometry || isPolygon) {
			boost::property_tree::ptree schemaData = placemark.get_child("ExtendedData").get_child("SchemaData");
			std::string labelName, ident;
			for (boost::property_tree::ptree::value_type const& simpleData : schemaData) {
				if (simpleData.first != "SimpleData") continue;
				std::string str(simpleData.second.get_child("<xmlattr>").get<std::string>("name"));

				if (str == "Upper_Limit" || str == "Top") topPresent = AirspaceConverter::ParseAltitude(simpleData.second.data(), true, airspace);
				else if (str == "Lower_Limit" || str == "Base") basePresent = AirspaceConverter::ParseAltitude(simpleData.second.data(), false, airspace);
				else if (str == "NAM" || str == "name" || str == "Name") labelName = simpleData.second.data();
				else if (str == "IDENT") ident = simpleData.second.data();
				else if (str == "Category") {
					unsigned int cat = Airspace::Type::CLASSA;
					bool found = false;
					do {
						if (simpleData.second.data() == Airspace::LongCategoryName((Airspace::Type)cat)) found = true;
						else cat++;
					} while (cat < Airspace::Type::UNDEFINED && !found);
					if (!found) {
						cat = Airspace::Type::CLASSA;
						 do {
							if (simpleData.second.data() == Airspace::CategoryName((Airspace::Type)cat)) found = true;
							else cat++;
						 } while (cat < Airspace::Type::UNDEFINED && !found);
					
					}
					if (found) category = (Airspace::Type)cat;
					else {
						if (simpleData.second.data() == "Danger") category = Airspace::Type::D;
						else if (simpleData.second.data() == "Prohibited") category = Airspace::Type::P;
						else if (simpleData.second.data() == "Restricted") category = Airspace::Type::R;
						else AirspaceConverter::LogError("Unable to parse airspace category in the label: " + simpleData.second.data());
					}				
				}
			}

			// If found a category from the label use it
			if (category != airspace.GetType()) airspace.SetType(category);

			// Remember the placemark name
			std::string placemarkName(airspace.GetName());

			// Consider the name from the label
			if (!labelName.empty()) {
				airspace.SetName(labelName);
				airspace.GuessClassFromName();
			}

			// The name from the label is valid
			if (!airspace.GetName().empty()) {

				// If ident is also present in the label use them joined as name
				if (!ident.empty() && airspace.GetName() != ident && !airspace.NameStartsWithIdent(ident)) airspace.SetName(ident.append(" ") + airspace.GetName());

				// Otherwise if a name from the tag is present and different from the label join them
				else if (!placemarkName.empty() && airspace.GetName() != placemarkName) airspace.SetName(placemarkName.append(" ") + airspace.GetName());
			}

			// No valid name from the label then use placemark name, add also ident if present
			else {
				airspace.SetName(placemarkName);
				if (!ident.empty() && ident != placemarkName && !airspace.NameStartsWithIdent(ident)) airspace.SetName(ident.append(" ") + placemarkName);
			}

			// If still invalid category skip it
			if (airspace.GetType() == Airspace::Type::UNDEFINED) return false;
		}

		// The name can be empty also after doing GuessClassFromName(), so make sure there is something there
		if (airspace.GetName().empty()) airspace.SetName(airspace.GetLongCategoryName());

		bool pointsFound(false);

		// If we expect a multigeometry...
		if(isMultiGeometry) {
			assert(!isPolygon && placemark.count("MultiGeometry") == 1);

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
							AirspaceConverter::LogWarning("skipping MultiGeometry with invalid base altitude: " + airspace.GetName());
							return false;
						}
					}
					if (!topPresent) {
						if (topFound) airspace.SetTopAltitude(top);
						else {
							AirspaceConverter::LogWarning("skipping MultiGeometry with invalid top altitude: " + airspace.GetName());
							return false;
						}
					}
				}
			}
		}

		// Otherwise it should be a single extruded polygon
		else if (isPolygon) {
			assert(!isMultiGeometry && placemark.count("Polygon") == 1);
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

		// Otherwise we process it as LineString
		else {
			assert(processLineString && !isMultiGeometry && !isPolygon && placemark.count("LineString") == 1);

			// Just get the coordinates
			double avgAltitude = 0;
			pointsFound = ProcessCoordinates(placemark.get_child("LineString"), airspace, avgAltitude);

			if (pointsFound) {
				if (airspace.GetType() == Airspace::Type::UNDEFINED) airspace.SetType(Airspace::Type::UNKNOWN);
				Altitude alt;
				alt.SetGND();
				airspace.SetBaseAltitude(alt);
				alt.SetAltMt(1000, false); // We put here a defualt altitude of 1000 m AGL
				airspace.SetTopAltitude(alt);
				AirspaceConverter::LogWarning("treating track as airspace: " + airspace.GetName());
			}
		}

		if (pointsFound) {
			// Check if the altitudes make sense
			if (airspace.GetType() != Airspace::Type::UNDEFINED && airspace.GetBaseAltitude() < airspace.GetTopAltitude()) {
				airspaces.insert(std::pair<int, Airspace>(airspace.GetType(), std::move(airspace)));
				return true;
			} else AirspaceConverter::LogWarning("skipping Placemark with invalid altitudes: " + airspace.GetName());
		}

	} catch (...) {}
	return false;
}

bool KML::ReadKML(const std::string& filename) {
	std::ifstream input(filename);
	if (!input.is_open() || input.bad()) {
		AirspaceConverter::LogError("Unable to open KML file: " + filename);
		return false;
	}
	AirspaceConverter::LogMessage("Reading KML file: " + filename);
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
		AirspaceConverter::LogError("Exception while parsing basic elements of KML file.");
		return false;
	}
	return true;
}
