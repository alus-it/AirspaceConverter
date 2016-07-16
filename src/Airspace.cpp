//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "Airspace.h"
#include <string>
#include <cmath>
#include <cassert>

const int LatLon::UNDEF_LAT = -91;
const int LatLon::UNDEF_LON = -181;
const double Altitude::FEET2METER = 0.3048; // 1 Ft = 0.3048 m
const double Altitude::K1 = 0.190263;
const double Altitude::K2 = 8.417286e-5;
const double Altitude::QNE = 1013.25;
double Altitude::QNH = QNE;

const double Geometry::PI = 3.14159265358979323846;
const double Geometry::TWO_PI = 2 * PI;
const double Geometry::PI_2 = PI / 2;
const double Geometry::DEG2RAD = PI / 180;
const double Geometry::RAD2DEG = 180 / PI;
const double Geometry::NM2RAD = PI / (180 * 60);
const double Geometry::RAD2NM = (180 * 60) / PI;

double Geometry::resolution = 0.3 * NM2RAD; // 0.3 NM = 555.6 m

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
	if (QNH != QNE) 
	{
		altMt = QNEaltitudeToQNHaltitude(altMt);
		altFt = (int)(altMt / FEET2METER);
	}
}

const std::string Altitude::ToString() const {
	if (fl > 0) return "FL " + std::to_string(fl);
	if (refIsMsl) {
		if(altFt!=0) return std::to_string(altFt) + " FT AMSL";
		else return "MSL";
	}
	if (IsGND()) return "GND";
	return std::to_string(altFt) + " FT AGL";
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
	false, //TMA
	false, //TMZ
	false, //RMZ
	false, //FIR
	false, //UIR
	false, //OTH
	false, //GLIDING
	false, //NOGLIDER
	false, //WAVE
	false //UNKNOWN
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
	"UNKNOWN" //UNKNOWN
};

Airspace::Airspace(Airspace&& orig) // Move constructor
	: top(std::move(orig.top))
	, base(std::move(orig.base))
	, geometries(std::move(orig.geometries))
	, points(std::move(orig.points))
	, type(std::move(orig.type))
	, name(std::move(orig.name))
{
	orig.type = UNKNOWN;
}

Airspace::~Airspace()
{
	for (const Geometry* g : geometries) delete g;
	geometries.clear();
}

void Airspace::Clear()
{
	type = UNKNOWN;
	name.clear();
	for (const Geometry* g : geometries) delete g;
	geometries.clear();
	points.clear();
}

void Airspace::AddPoint(const double & lat, const double & lon)
{
	geometries.push_back(new Point(LatLon(lat, lon)));
}

void Airspace::AddGeometry(const Geometry* geometry)
{
	geometries.push_back(geometry);
}

void Airspace::Discretize()
{
	points.clear();
	for (auto g : geometries) g->Discretize(points);
}

double Geometry::AbsAngle(const double& angle) { //to put angle in the range between 0 and 2PI
	if (isinf(angle)) return TWO_PI;
	double absangle = std::fmod(angle, TWO_PI);
	if (absangle<0) absangle += TWO_PI;
	return absangle;
}

double Geometry::CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& d) { //not require pre-computation of distance
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	if (lat2 == PI_2) return TWO_PI; //we are going to N pole
	if (lat2 == -PI_2) return PI; //we are going to S pole
	if (lon1 == lon2) return lat1>lat2 ? PI : TWO_PI; //we are going to E or W
	return AbsAngle(sin(lon2 - lon1) < 0 ? acos((sin(lat2) - sin(lat1)*cos(d)) / (sin(d)*cos(lat1))) : TWO_PI - acos((sin(lat2) - sin(lat1)*cos(d)) / (sin(d)*cos(lat1))));
}

double Geometry::CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2) { //not require pre-computation of distance
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	if (lat2 == PI_2) return TWO_PI; //we are going to N pole
	if (lat2 == -PI_2) return PI; //we are going to S pole
	if (lon1 == lon2) return lat1>lat2 ? PI : TWO_PI; //we are going to E or W
	return AbsAngle(-atan2(sin(lon1 - lon2)*cos(lat2), cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lon1 - lon2)));
}

double Geometry::CalcAngularDist(const double& lat1, const double& lon1, const double& lat2, const double& lon2) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	//return acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon1 - lon2)); // faster but with more rounding error
	return 2 * asin(sqrt(pow(sin((lat1 - lat2) / 2), 2) + cos(lat1) * cos(lat2) * pow(sin((lon1 - lon2) / 2), 2)));
}

LatLon Geometry::CalculateRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(dir > -PI && dir <= 2 * TWO_PI);
	const double lat = asin(sin(lat1) * cos(dst) + cos(lat1) * sin(dst) * cos(dir));
	const double lon = lon1 + atan2(sin(dir) * sin(dst) * cos(lat1), cos(dst) - sin(lat1) * sin(lat));
	return LatLon(lat * RAD2DEG, lon * RAD2DEG);
}

bool Point::Discretize(std::vector<LatLon>& output) const
{
	output.push_back(point); // Here it's easy :)
	return true;
}

Sector::Sector(const double& clat, const double& clon, const double& radiusNM, const double& dir1, const double& dir2, const bool& clockwise)
	: Geometry(LatLon(clat, clon))
	, radius(radiusNM * NM2RAD)
	, angleStart(dir1 * DEG2RAD)
	, angleEnd(dir2 * DEG2RAD)
	, couterclockwise(clockwise)
	, latc(clat * DEG2RAD)
	, lonc(clon * DEG2RAD) {
}

Sector::Sector(const double& clat, const double& clon, const double& lat1, const double& lon1, const double& lat2, const double& lon2, const bool& clockwise)
	: Geometry(LatLon(clat, clon))
	, couterclockwise(clockwise)
	, latc(clat * DEG2RAD)
	, lonc(clon * DEG2RAD)
{
	const double lat1r = lat1 * DEG2RAD;
	const double lon1r = lon1 * DEG2RAD;
	const double lat2r = lat2 * DEG2RAD;
	const double lon2r = lon2 * DEG2RAD;
	radius = CalcAngularDist(latc, lonc, lat1r, lon1r);
	
	#ifdef _DEBUG
	assert(radius > 0);
	const double dist2 = CalcAngularDist(latc, lonc, lat2r, lon2r) * RAD2NM;
	assert(fabs((radius * RAD2NM) - dist2) < 0.2);
	#endif
	
	angleStart = CalcGreatCircleCourse(latc, lonc, lat1r, lon1r);
	angleEnd = CalcGreatCircleCourse(latc, lonc, lat2r, lon2r);
}

double Geometry::FindStep(const double& radius, const double& angle) {
	assert(angle >= 0 && angle <= TWO_PI);
	assert(radius >= 0); 
	static const double smallRadius = 3 * NM2RAD; // 3 NM, radius under it the number of points will be decreased
	static const double m = 300 / smallRadius; // coeffcient to decrease points for small circles, 300 is default number of points for circles bigger than 3 NM
	int steps;
	if (radius < smallRadius) {
		steps = (int)((angle * (m * radius + 8)) / TWO_PI); // 8 is the minimum number of points for circles with radius -> 0
	} else steps = (int)((angle * radius) / resolution);
	return angle / steps;
}

bool Sector::Discretize(std::vector<LatLon>& output) const
{
	if (couterclockwise) {
		double e = angleStart <= angleEnd ? angleEnd : angleEnd + TWO_PI;
		const double step = FindStep(radius, AbsAngle(e - angleStart));
		assert(angleStart <= e);
		for (double a = angleStart; a < e; a += step)
			output.push_back(CalculateRadialPoint(latc, lonc, a, radius));
	} else {
		const double s = angleStart >= angleEnd ? angleStart : angleStart + TWO_PI;
		const double step = FindStep(radius, AbsAngle(s - angleEnd));
		assert(s >= angleEnd);
		for (double a = s; a > angleEnd; a -= step)
			output.push_back(CalculateRadialPoint(latc, lonc, a, radius));
	}
	return true;
}

Circle::Circle(const double& lat, const double& lon, const double& radiusNM)
	: Geometry(LatLon(lat, lon))
	, radius(radiusNM * NM2RAD)
	, latc(lat * DEG2RAD)
	, lonc(lon * DEG2RAD) {
}

bool Circle::Discretize(std::vector<LatLon>& output) const
{
	const double step = FindStep(radius, TWO_PI);
	for (double a = 0; a < TWO_PI; a += step) output.push_back(CalculateRadialPoint(latc, lonc, a, radius));
	return true;
}

/* Airway not supported yet
bool AirwayPoint::Discretize(std::vector<LatLon>& output) const
{
	// TODO...
	return false;
}

*/
