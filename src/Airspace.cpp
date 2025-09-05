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

#include "Airspace.hpp"
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>

const bool Airspace::CATEGORY_VISIBILITY[Airspace::UNDEFINED] = {
	false,	//CLASSA
	false,	//CLASSB
	false,	//CLASSC
	false,	//CLASSD
	false,	//CLASSE
	false,	//CLASSF
	false,	//CLASSG
	true,	//DANGER
	true,	//PROHIBITED
	true,	//RESTRICTED
	true,	//CTR
	true,	//TMZ
	true,	//RMZ
	true,	//GLIDING
	true,	//NOGLIDER
	true,	//WAVE
	true,	//NOTAM
	false,	//OTHER
	true,	//TMA
	false,	//FIR
	false,	//UIR
	false,	//OTH
	false,	//AWY
	true,	//MATZ
	false,	//MTMA
	false,	//MTRA
	false,	//TFR
	false,	//ADA
	false,	//ADIZ
	true,	//CTA
	false,	//DFIR
	false,	//TIZ
	false,	//TIA
	true,	//SRZ
	true,	//ATZ
	false,	//FISA
	false,	//MBZ
	true,	//ASR
	true,	//COMP
	false,	//TRZ
	false,	//VFRR
	false,	//RTZ
	true,	//PARA
	true,	//LFZ
	false,	//CFZ
	false,	//MOA
	false,	//MTA
	false,	//TSA
	false,	//TRA
	false	//UNKNOWN
};

const std::string Airspace::CATEGORY_NAMES[Airspace::UNDEFINED] = {
	"Class A",
	"Class B",
	"Class C",
	"Class D",
	"Class E",
	"Class F",
	"Class G",
	"D",
	"P",
	"R",
	"CTR",
	"TMZ",
	"RMZ",
	"Gliding",
	"No Glider",
	"Wave",
	"NOTAM",
	"OTHER",
	"TMA",
	"FIR",
	"UIR",
	"OTH",
	"AWY",
	"MATZ",
	"MTMA",
	"MTRA",
	"TFR",
	"ADA",
	"ADIZ",
	"CTA",
	"DFIR",
	"TIZ",
	"TIA",
	"SRZ",
	"ATZ",
	"FISA",
	"MBZ",
	"ASR",
	"COMP",
	"TRZ",
	"VFRR",
	"RTZ",
	"PARA",
	"LFZ",
	"CFZ",
	"MOA",
	"MTA",
	"TSA",
	"TRA",
	"UNKNOWN"
};

const std::string Airspace::LONG_CATEGORY_NAMES[Airspace::UNDEFINED] = {
	"Airspace class A",
	"Airspace class B",
	"Airspace class C",
	"Airspace class D",
	"Airspace class E",
	"Airspace class F",
	"Airspace class G",
	"Danger area",
	"Prohibited area",
	"Restricted area",
	"Control Traffic Region",
	"Transponder Mandatory Zone",
	"Radio Mandatory Zone",
	"Gliding area",
	"No gliding area",
	"Wave window",
	"Notice to Airmen",
	"Other airspace type",
	"Terminal Manoeuvring Area",
	"Flight Information Region",
	"Upper Information Region",
	"Over The Horizon",
	"Airway",
	"Military Aerodrome Traffic Zone",
	"Military Terminal Zone",
	"Military Temporary Reserved Area",
	"Temporary Flight Restriction",
	"Advisory Area",
	"Air Defense Identification Zone",
	"Control Area",
	"Delegated FIR",
	"Traffic Information Zone",
	"Traffic Information Area",
	"Special Rules Zone",
	"Aerodrome Traffic Zone",
	"Flight Information Service Area",
	"Mandatory Broadcast Zone",
	"Aerial Sporting and Recreation Area",
	"Competition boundary",
	"Transponder Recommended Zone",
	"VFR Route",
	"Radio/Transponder Mandatory zone",
	"Parachute jumping area",
	"Low Flying Zone",
	"Common Frequency Zone",
	"Military Operating Area",
	"Military Training Area",
	"Temporary Segregated Airspace",
	"Temporary Reserved Airspace",
	"Unknown"
};

Airspace::Airspace(Type category)
	: type(category)
	, airspaceClass(category >= CLASSA && category <= CLASSG ? category : UNDEFINED)
	, transponderCode(-1) {
}

Airspace::Airspace(const Airspace& orig) // Copy constructor
	: top(orig.top)
	, base(orig.base)
	, geometries(orig.geometries)
	, points(orig.points)
	, type(orig.type)
	, airspaceClass(orig.airspaceClass)
	, name(orig.name)
	, radioFrequencies(orig.radioFrequencies)
	, transponderCode(orig.transponderCode) {
}

Airspace::Airspace(Airspace&& orig) // Move constructor
	: top(std::move(orig.top))
	, base(std::move(orig.base))
	, geometries(std::move(orig.geometries))
	, points(std::move(orig.points))
	, type(std::move(orig.type))
	, airspaceClass(std::move(orig.airspaceClass))
	, name(std::move(orig.name))
	, radioFrequencies(std::move(orig.radioFrequencies))
	, transponderCode(std::move(orig.transponderCode)) {
	orig.type = UNDEFINED;
}

Airspace& Airspace::operator=(const Airspace& other) {
	top = other.top;
	base = other.base;
	geometries = other.geometries;
	points = other.points;
	type = other.type;
	airspaceClass = other.airspaceClass;
	name = other.name;
	radioFrequencies = other.radioFrequencies;
	transponderCode = other.transponderCode;
	return *this;
}

bool Airspace::operator==(const Airspace& other) const {
	if (top != other.top) return false;
	if (base != other.base) return false;
	if (type != other.type) return false;
	if (airspaceClass != other.airspaceClass) return false;
	return points == other.points;
}

Airspace::~Airspace() {
	ClearGeometries();
}

void Airspace::SetType(const Type& category) {
	type = category;
	airspaceClass = category >= CLASSA && category <= CLASSG ? category : UNDEFINED;
}

void Airspace::SetClass(const Type& airspClass) {
	if(airspClass > CLASSG) return;
	airspaceClass = airspClass;
	if (type <= CLASSG && type != airspClass) type = airspClass;
}

void Airspace::AddRadioFrequency(const int frequencyHz, const std::string& description) {
	assert(frequencyHz > 0);
	radioFrequencies.push_back(std::make_pair(frequencyHz,description)); // here we expect aready validated airband radio frequencies
}

bool Airspace::SetTransponderCode(const std::string& code) {
	if (code.empty() || code.length() > 4) return false;
	for (char c : code) {
		if (c < '0' || c > '7') return false;
	}
	try {
		transponderCode = (short)std::stoi(code, 0, 8);
		return true;
	} catch ( ... ) {}
	return false;
}

std::string Airspace::GetTransponderCode() const {
	std::ostringstream ss;
	ss << std::setw(4) << std::setfill('0') << std::oct << transponderCode;
	return ss.str();
}

bool Airspace::GuessClassFromName() {
	if (type != CTR && type != TMA && type != UNDEFINED) return false;
	if (name.empty()) return false;
	Type foundClass = UNDEFINED;
	const static std::vector<std::string> keywords = {
		"Airspace class",
		"Luftraumklasse",
		"class",
		"Class",
		"CLASS",
		"klasse",
		"Klasse",
		"KLASSE",
		"classe",
		"Classe",
		"CLASSE"
	};
	std::string::size_type nameLength = name.length();
	std::string::size_type start = 0, length = 0;
	for(const std::string& keyword : keywords) {
		start = name.find(keyword);
		if(start == std::string::npos) continue;
		std::string::size_type pos = start + keyword.length();
		if (nameLength <= pos) continue;
		char c = name.at(pos);
		if (c == ' ' || c == ':') {
			pos++;
			if (nameLength <= pos) continue;
			c = name.at(pos);
			if (c == ' ' || c == ':') {
				pos++;
				if (nameLength <= pos) continue;
				c = name.at(pos);
			}
		}
		if (c >= 'A' && c <= 'F') {
			foundClass = (Type)(c - 'A');
			length = pos - start + 1;
			break;
		} else if (c >= 'a' && c <= 'f') {
			foundClass = (Type)(c - 'a');
			length = pos - start + 1;
			break;
		}
	}

	if (foundClass == UNDEFINED) return false;

	assert(foundClass >= CLASSA && foundClass <= CLASSG);
	airspaceClass = foundClass;
	if (type == UNDEFINED) type = foundClass;
	
	// Remove the text "Class: C" from the name
	name.erase(start, length);

	// Remove the eventual dash
	if (name.length() >=3 && name.compare(name.length() - 3, 3, " - ") == 0) name.erase(name.length() - 3, 3);

	return true;
}

bool Airspace::NameStartsWithIdent(const std::string& ident) {
	if(name.length() < 4 || ident.length() < 4) return false;
	return(ident.find(name.substr(0,4)) != std::string::npos);
}

void Airspace::Clear() {
	type = UNDEFINED;
	airspaceClass = UNDEFINED;
	name.clear();
	ClearPoints();
	radioFrequencies.clear();
	transponderCode = -1;
}

void Airspace::ClearPoints() {
	ClearGeometries();
	points.clear();
}

void Airspace::ClearGeometries() {
	if (geometries.empty()) return;
	for (const Geometry* g : geometries) delete g;
	geometries.clear();
}

bool Airspace::AddPoint(const Geometry::LatLon& point) {
	// Make sure the point is not a duplicate of the last, not necessary to add it
	if (!points.empty() && points.back() == point) return false;

	// Make the new single "Point" geometry
	geometries.push_back(new Point(point));

	// Add the point
	points.push_back(point);

	return true;
}

bool Airspace::AddPointLatLonOnly(const double& lat, const double& lon) {
	const Geometry::LatLon point(lat, lon);

	// Make sure the point is not a duplicate or very similar to the last, not necessary to add it
	if (!points.empty() && point.IsAlmostEqual(points.back())) return false;

	// Add the point
	points.push_back(point);

	return true;
}

bool Airspace::AddPointGeometryOnly(const Geometry::LatLon& point) {
	// Make sure the point is not a duplicate of the last, not necessary to add it
	if (!geometries.empty() && geometries.back()->IsPoint() && geometries.back()->GetCenterPoint() == point) return false;
	
	// Make the new single "Point" geometry
	geometries.push_back(new Point(point));

	return true;
}

bool Airspace::ArePointsValid() const {
	// The number of points must be at least 3+1 (plus the closing one)
	assert(points.size() > 3);
	
	// Check if it is closed
	if (points.front() != points.back()) return false;

	// Except last one the points must be different from each other
	const size_t l = points.size() - 1;

	// Using a set with O(N log N) is faster in the generic case, but here this way should be faster in most of cases and requiring less memory
	for (size_t i = 0; i < l; i++) for (size_t j = i+1; j < l; j++) {
		if (points.at(i) == points.at(j)) return false;
	}
	
	// If we arrived here it is all OK
	return true;
}

void Airspace::RemoveTooCloseConsecutivePoints() {
	if (points.size() < 2) return;
	auto prevPoint = points.begin();
	auto it = prevPoint + 1; // second element
	while (it != points.end()) {
		if ((*it).IsAlmostEqual(*prevPoint)) it = points.erase(it);
		else {
			prevPoint = it;
			it++;
		}
	}
}

bool Airspace::ClosePoints() {
	// Here we expect at least 3 points
	if(points.size() < 3) return false;

	// Make sure that the last point in the vector is equal to the first so "closing" the polygon
	const Geometry::LatLon& first = points.front();
	if (first != points.back()) points.push_back(first);

	// Check for repeated points or equal to first
	std::vector<Geometry::LatLon>::iterator it = std::next(points.begin()); // Start from second point
	while(it != std::prev(points.end())) { // until the point before last one
		if (first == (*it) || (*std::prev(it)) == (*it)) it = points.erase(it);
		else it++;
	}

	// For a valid closed polygon we need at least 3 points plus closing point
	return points.size() > 3;
}

void Airspace::AddGeometry(const Geometry* geometry) {
	assert(geometry != nullptr);
	geometries.push_back(geometry);
	geometry->Discretize(points);
}

void Airspace::EvaluateAndAddArc(std::vector<Geometry::LatLon*>& arcPoints, std::vector<std::pair<const double, const double>>& centerPoints, const bool& clockwise) {
	if (arcPoints.size() > 4) geometries.push_back(new Sector(Geometry::AveragePoints(centerPoints), *arcPoints.front(), *arcPoints.back(), clockwise));
	else for(const Geometry::LatLon* p : arcPoints) {
			AddPointGeometryOnly(*p);
	}
	arcPoints.clear();
	centerPoints.clear();
}

void Airspace::EvaluateAndAddCircle(const std::vector<Geometry::LatLon*>& arcPoints, const std::vector<std::pair<const double, const double>>& centerPoints) {
	if (arcPoints.size() > 10) {
		
		// Find center point
		const Geometry::LatLon center(Geometry::AveragePoints(centerPoints));

		// Calculate radius rounded in NM
		const double radius(Geometry::RoundDistanceInNM(Geometry::AverageRadius(center, arcPoints)));

		//TODO: Eventually check if it not a too small radius
		//if (radius > 0.003) { // 0.003 NM = 5.556 m

		// Finally add the so resulting circle
		geometries.push_back(new Circle(center, radius));
	} else for (const Geometry::LatLon* p : arcPoints) AddPointGeometryOnly(*p);
}

bool Airspace::Undiscretize() {
	if (!geometries.empty()) return true;
	if (points.empty()) return false;
	assert(points.size() >= 4);
	const size_t steps = points.size() - 2;
	std::vector<Geometry::LatLon*> arcPoints;
	std::vector<std::pair<const double, const double>> centerPoints;
	bool alreadyOnArc = false;
	bool alwaysOnSameArc = false;
	bool isClockwise;
	double prevRadius = 0;
	for (size_t a = 0, b = 1, c = 2; a < steps; a++, b++, c++) {
		double latc = -7, lonc = -7, radius = 0;
		bool clockwise;
		if (Geometry::ArePointsOnArc(points.at(a), points.at(b), points.at(c), latc, lonc, radius, clockwise)) {
			if (alreadyOnArc) { // the arc seems to continue
				assert(prevRadius != 0);
				const double smallDst = std::min(radius, prevRadius) / 10; // Find a small distance to compare with

				// If the radius is similar to the previous and the new center is enough near to the previous consider this as the same arc
				if (std::fabs(radius-prevRadius) < smallDst && Geometry::CalcAngularDist(latc, lonc, centerPoints.back().first, centerPoints.back().second) < smallDst) {
					arcPoints.push_back(&points.at(c));
					centerPoints.emplace_back(latc, lonc);
					prevRadius = radius;
				} else { // Not on the same arc but another new one
					assert(a > 0);
					assert(!arcPoints.empty());
					assert(!centerPoints.empty());
					EvaluateAndAddArc(arcPoints, centerPoints, isClockwise);
					alreadyOnArc = false;
					prevRadius = 0;
					alwaysOnSameArc = false;
				}
			} else { // New arc
				assert(arcPoints.empty());
				assert(centerPoints.empty());
				arcPoints.reserve(3);
				arcPoints.push_back(&points.at(a));
				arcPoints.push_back(&points.at(b));
				arcPoints.push_back(&points.at(c));
				centerPoints.emplace_back(latc, lonc);
				isClockwise = clockwise;
				alreadyOnArc = true;
				prevRadius = radius;
				if (a == 0) alwaysOnSameArc = true;
			}
		} else {
			if (alreadyOnArc) {
				alreadyOnArc = false;
				alwaysOnSameArc = false;
				EvaluateAndAddArc(arcPoints, centerPoints, isClockwise);
				prevRadius = 0;
			} else AddPointGeometryOnly(points.at(a));
		}
	}
	if (alreadyOnArc) { // Add the remaining curve to geometries
		assert(prevRadius > 0);
		if (alwaysOnSameArc) EvaluateAndAddCircle(arcPoints, centerPoints);	// If we were always on arc then here we have a circle
		else EvaluateAndAddArc(arcPoints, centerPoints, isClockwise);
	} else { // Otherwise add the remaining 2 points
		if (!geometries.empty() && geometries.back()->GetCenterPoint() != points.at(steps)) AddPointGeometryOnly(points.at(steps));
		if (!geometries.empty() && geometries.back()->GetCenterPoint() != points.at(steps+1)) AddPointGeometryOnly(points.at(steps+1));
	}
	return true;
}

bool Airspace::IsWithinLatLonLimits(const Geometry::Limits& limits) const {
	bool pointWhithinLimitsFound(false);
	for(const Geometry::LatLon& pos : points) if (limits.IsPositionWithinLimits(pos)) {
		pointWhithinLimitsFound = true;
		break;
	}
	return pointWhithinLimitsFound;
}

bool Airspace::IsWithinAltLimits(const Altitude& floor, const Altitude& ceil) const {
	if (top < ceil && top > floor) return true;
	if (base > floor && base < ceil) return true;
	return false;
}

void Airspace::CalculateSurface(double& area, double& perimeter) const {
	// Create geographic polygon
	boost::geometry::model::polygon<boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree> > > polygon;
	for (const Geometry::LatLon& point : points) boost::geometry::append(polygon, boost::make_tuple(point.Lon(), point.Lat()));

	// Geographic strategy with WGS84 spheroid (spheroid sizes in Km)
	static const boost::geometry::strategy::area::geographic<> wgs84(boost::geometry::srs::spheroid<double>(6378.137, 6356.7523142451793));

	// Geographic strategy with Vincenty algorithm
	//static const boost::geometry::strategy::area::geographic<boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree >>, boost::geometry::formula::vincenty_inverse> vincenty;

	area = std::fabs(boost::geometry::area(polygon, wgs84)); // [Km2]
	perimeter = boost::geometry::perimeter(polygon) / 1000; // [Km]
}
