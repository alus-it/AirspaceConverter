//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "Airspace.h"
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <boost/version.hpp>
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4127 )
#include <boost/geometry/formulas/differential_quantities.hpp>
#include <boost/geometry/formulas/meridian_inverse.hpp>
#pragma warning( pop )
#endif
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>

const double Altitude::FEET2METER = 0.3048; // 1 Ft = 0.3048 m
const double Altitude::K1 = 0.190263;
const double Altitude::K2 = 8.417286e-5;
const double Altitude::QNE = 1013.25;
double Altitude::QNH = QNE;

/*Altitude::Altitude(const int FL) :
	refIsMsl(true),
	fl(FL),
	altFt(FL * 100),
	altMt(altFt * FEET2METER),
	isUnlimited(false) {
	if (QNH != QNE) {
		altMt = QNEaltitudeToQNHaltitude(altMt);
		altFt = (int)(altMt / FEET2METER);
	}
}*/

double Altitude::QNEaltitudeToStaticPressure(const double alt) {
	return std::pow((std::pow(QNE, K1) - K2*alt), 1.0 / K1);
}

double Altitude::StaticPressureToQNHaltitude(const double ps) {
	return (std::pow(QNH, K1) - std::pow(ps, K1)) / K2;
}

double Altitude::QNEaltitudeToQNHaltitude(const double ps) {
	return StaticPressureToQNHaltitude(QNEaltitudeToStaticPressure(ps));
}

void Altitude::SetFlightLevel(const int FL) {
	fl = FL;
	refIsMsl = true;
	altFt = FL * 100;
	altMt = altFt * FEET2METER;
	if (QNH != QNE) {
		altMt = QNEaltitudeToQNHaltitude(altMt);
		altFt = (int)(altMt / FEET2METER);
	}
}

const std::string Altitude::ToString() const {
	if (isUnlimited) return "UNLIMITED";
	if (fl > 0) return "FL " + std::to_string(fl);
	if (refIsMsl) {
		if(altFt!=0) return std::to_string(altFt) + " FT AMSL";
		else return "MSL";
	}
	if (IsGND()) return "GND";
	return std::to_string(altFt) + " FT AGL";
}

bool Altitude::operator<(const Altitude& other) const {
	if (isUnlimited && !other.isUnlimited) return false;
	if (!isUnlimited && other.isUnlimited) return true;
	return altMt < other.altMt;
}

bool Altitude::operator>(const Altitude& other) const {
	if (isUnlimited && !other.isUnlimited) return true;
	if (!isUnlimited && other.isUnlimited) return false;
	return altMt > other.altMt;
}

bool Altitude::operator<=(const Altitude& other) const {
	if (isUnlimited && !other.isUnlimited) return false;
	if (!isUnlimited && other.isUnlimited) return true;
	return altMt <= other.altMt;
}

bool Altitude::operator>=(const Altitude& other) const {
	if (isUnlimited && !other.isUnlimited) return true;
	if (!isUnlimited && other.isUnlimited) return false;
	return altMt >= other.altMt;
}

bool Altitude::operator==(const Altitude& other) const {
	if (isUnlimited == other.isUnlimited) return true;
	return altFt == other.altFt && refIsMsl == other.refIsMsl && fl == other.fl;
}

bool Altitude::operator!=(const Altitude& other) const {
	if (isUnlimited && other.isUnlimited) return false;
	return altFt != other.altFt || refIsMsl != other.refIsMsl || fl != other.fl;
}

const bool Airspace::CATEGORY_VISIBILITY[] = {
	false, //CLASSA
	false, //CLASSB
	false, //CLASSC
	false, //CLASSD
	false, //CLASSE
	false, //CLASSF
	false, //CLASSG
	true, //DANGER
	true, //PROHIBITED
	true, //RESTRICTED
	true, //CTR
	true, //TMA
	false, //TMZ
	false, //RMZ
	false, //FIR
	false, //UIR
	false, //OTH
	false, //GLIDING
	false, //NOGLIDER
	false, //WAVE
	false, //UNKNOWN
	false  //UNDEFINED
};

const std::string Airspace::CATEGORY_NAMES[] = {
	"A", //CLASSA
	"B", //CLASSB
	"C", //CLASSC
	"D", //CLASSD
	"E", //CLASSE
	"F", //CLASSF
	"G", //CLASSG
	"Danger", //DANGER
	"Prohibited", //PROHIBITED
	"Restricted", //RESTRICTED
	"CTR", //CTR
	"TMA", //TMA
	"TMZ", //TMZ
	"RMZ", //RMZ
	"FIR", //FIR
	"UIR", //UIR
	"OTH", //OTH
	"Gliding area", //GLIDING
	"No glider", //NOGLIDER
	"Wave window", //WAVE
	"Unknown", //UNKNOWN
	"UNDEFINED" //UNDEFINED
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
	radioFrequencies.push_back(std::make_pair(frequencyHz,description)); // here we expect alredy validated airband radio frequencies
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

void Airspace::AddPoint(const Geometry::LatLon& point) {
	geometries.push_back(new Point(point));
	points.push_back(point);
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
	else for(const Geometry::LatLon* p : arcPoints) geometries.push_back(new Point(*p));
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
	} else for (const Geometry::LatLon* p : arcPoints) geometries.push_back(new Point(*p));
}

bool Airspace::Undiscretize() {
	if (!geometries.empty()) return true;
	if (points.empty()) return false;
	assert(points.size() >= 4);
	const unsigned int steps = (unsigned int)points.size() - 2;
	std::vector<Geometry::LatLon*> arcPoints;
	std::vector<std::pair<const double, const double>> centerPoints;
	bool alreadyOnArc = false;
	bool alwaysOnSameArc = false;
	bool isClockwise;
	double prevRadius = 0;
	for (unsigned int i = 0; i < steps; i++) {
		double latc = -7, lonc = -7, radius = 0;
		bool clockwise;
		if (Geometry::ArePointsOnArc(points.at(i), points.at(i + 1), points.at(i + 2), latc, lonc, radius, clockwise)) {
			if (alreadyOnArc) { // the arc seems to continue
				assert(prevRadius != 0);
				const double smallDst = std::min(radius, prevRadius) / 10; // Find a small distance to compare with

				// If the radius is similar to the previous and the new center is enough near to the previous consider this as the same arc
				if (std::fabs(radius-prevRadius) < smallDst && Geometry::CalcAngularDist(latc, lonc, centerPoints.back().first, centerPoints.back().second) < smallDst) {
					arcPoints.push_back(&points.at(i + 2));
					centerPoints.emplace_back(latc, lonc);
					prevRadius = radius;
				} else { // Not on the same arc but another new one
					assert(i > 0);
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
				arcPoints.push_back(&points.at(i));
				arcPoints.push_back(&points.at(i + 1));
				arcPoints.push_back(&points.at(i + 2));
				centerPoints.emplace_back(latc, lonc);
				isClockwise = clockwise;
				alreadyOnArc = true;
				prevRadius = radius;
				if (i == 0) alwaysOnSameArc = true;
			}
		} else {
			if (alreadyOnArc) {
				alreadyOnArc = false;
				alwaysOnSameArc = false;
				EvaluateAndAddArc(arcPoints, centerPoints, isClockwise);
				prevRadius = 0;
			} else geometries.push_back(new Point(points.at(i)));
		}
	}
	if (alreadyOnArc) { // Add the remaining curve to geometries
		assert(prevRadius > 0);
		if (alwaysOnSameArc) EvaluateAndAddCircle(arcPoints, centerPoints);	// If we were always on arc then here we have a circle
		else EvaluateAndAddArc(arcPoints, centerPoints, isClockwise);
	} else { // Otherwise add the remaining 2 points
		geometries.push_back(new Point(points.at(steps)));
		geometries.push_back(new Point(points.at(steps+1)));
	}
	return true;
}

bool Airspace::IsWithinLimits(const Geometry::Limits& limits) const {
	bool pointWhithinLimitsFound(false);
	for(const Geometry::LatLon& pos : points) if (limits.IsPositionWithinLimits(pos)) {
		pointWhithinLimitsFound = true;
		break;
	}
	return pointWhithinLimitsFound;
}

void Airspace::CalculateSurface(double& area, double& perimeter) const {
#if BOOST_VERSION >= 106700 // Spheroidal
	// Create geographic polygon
	boost::geometry::model::polygon<boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree> > > polygon;
	for (const Geometry::LatLon& point : points) boost::geometry::append(polygon, boost::make_tuple(point.Lon(), point.Lat()));

	// Geographic strategy with WGS84 spheroid (spheroid sizes in Km)
	static const boost::geometry::strategy::area::geographic<> wgs84(boost::geometry::srs::spheroid<double>(6378.137, 6356.7523142451793));

	// Geographic strategy with Vincenty algorithm
	//static const boost::geometry::strategy::area::geographic<boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree >>, boost::geometry::formula::vincenty_inverse> vincenty;

	area = std::fabs(boost::geometry::area(polygon, wgs84)); // [Km2]
	perimeter = boost::geometry::perimeter(polygon) / 1000; // [Km]

#else // Spherical
	// Create spherical polygon
	boost::geometry::model::polygon<boost::geometry::model::point<float, 2, boost::geometry::cs::spherical_equatorial<boost::geometry::degree> > > polygon;
	for (const Geometry::LatLon& point : points) boost::geometry::append(polygon, boost::make_tuple(point.Lon(), point.Lat()));
	
	static const double earthRadiusKm = 6371.0088;
	static const double squareEarthRadiusKm = pow(earthRadiusKm,2);

	// Spherical strategy with mean Earth radius in Km
	//static const boost::geometry::strategy::area::spherical<> sphericalStrategy(earthRadiusKm);

	//area = std::fabs(boost::geometry::area(polygon, sphericalStrategy));
	area = std::fabs(boost::geometry::area(polygon)) * squareEarthRadiusKm; // [Km2]
	perimeter = (double)boost::geometry::perimeter(polygon) * earthRadiusKm; // [Km]
#endif
}

