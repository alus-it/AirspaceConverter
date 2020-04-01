//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Authors     : Alberto Realis-Luc <alberto.realisluc@gmail.com>
//               Valerio Messina <efa@iol.it>
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Web         : https://www.alus.it/AirspaceConverter
// Copyright   : (C) 2016-2020 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "Geometry.h"
#include "OpenAir.h"
#include <cmath>
#include <cassert>

const int Geometry::LatLon::UNDEF_LAT = -91;
const int Geometry::LatLon::UNDEF_LON = -181;
const double Geometry::LatLon::SIXTY = 60;
const double Geometry::LatLon::DEGTOL = 0.00005 * (1.0 / 60); //0.00005 min = 0.0926 m
const double Geometry::PI = 3.1415926535897932384626433832795;
const double Geometry::TWO_PI = PI * 2;
const double Geometry::PI_2 = PI / 2;
const double Geometry::DEG2RAD = PI / 180;
const double Geometry::RAD2DEG = 180.0 / PI;
const double Geometry::NM2RAD = PI / (180 * 60);
const double Geometry::RAD2NM = (180.0 * 60) / PI;
const double Geometry::NM2M = 1852.0;
const double Geometry::MI2M = 1609.344;
const double Geometry::M2RAD = NM2RAD / NM2M;

const double Geometry::TOL = 2e-10;

double Geometry::resolution = 0.3 * NM2RAD; // 0.3 NM = 555.6 m

void Geometry::LatLon::convertDec2DegMin(const double& dec, int& deg, double& min) {
	const double decimal = std::fabs(dec);
	deg = (int)std::floor(decimal);
	min = (decimal-deg)*SIXTY;
	if (std::fabs(min) <= TOL) min = 0;
}

void Geometry::LatLon::convertDec2DegMinSec(const double& dec, int& deg, int& min, int& sec) {
	double decimal = std::fabs(dec);
	deg = (int)std::floor(decimal);
	decimal = (decimal-deg)*SIXTY;
	min = (int)std::floor(decimal);
	sec = (int)std::round((decimal-min)*SIXTY);
	if (sec == 60) {
		min++;
		sec = 0;
	}
	if (min == 60) {
		deg++;
		min = 0;
	}
}

bool Geometry::LatLon::autoConvertDec2DegMinSec(const double& dec, int& deg, double& decimalMin, int& min, int& sec) {
	double decimal = std::fabs(dec);
	deg = (int)std::floor(decimal);
	decimalMin = (decimal-deg)*SIXTY;
	min = (int)std::floor(decimalMin);
	decimal = (decimalMin-min)*SIXTY;
	sec = (int)std::round(decimal);
	if (decimal - sec <= 0.05) { // 0.05 sec = 1.5433 m
		if (sec == 60) {
			min++;
			sec = 0;
		}
		if (min == 60) {
			deg++;
			min = 0;
		}
		return true;
	}
	if (std::fabs(decimalMin) <= TOL) decimalMin = 0;
	return false;
}

bool Geometry::LatLon::IsAlmostEqual(const LatLon& other) const {
	if (*this == other) return true;
	return std::fabs(lat-other.lat) < DEGTOL && std::fabs(lon-other.lon) < DEGTOL;
}

bool Geometry::Limits::Set(const LatLon& topLeftLimit, const LatLon& bottomRightLimit) {
	assert(topLeftLimit.IsValid());
	assert(bottomRightLimit.IsValid());
	topLeft = topLeftLimit;
	bottomRight = bottomRightLimit;
	Verify();
	return valid;
}

bool Geometry::Limits::Set(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon) {
	assert(LatLon::IsValidLat(topLat));
	assert(LatLon::IsValidLat(bottomLat));
	assert(LatLon::IsValidLon(leftLon));
	assert(LatLon::IsValidLon(rightLon));
	topLeft.SetLat(topLat);
	bottomRight.SetLat(bottomLat);
	topLeft.SetLon(leftLon);
	bottomRight.SetLon(rightLon);
	Verify();
	return valid;
}

void Geometry::Limits::Verify() {
	valid = topLeft.IsValid() && bottomRight.IsValid() && topLeft.Lat() > bottomRight.Lat() && topLeft.Lat() != bottomRight.Lat() && topLeft.Lon() != bottomRight.Lon();
	if (valid) acrossAntiGreenwich = topLeft.Lon() > bottomRight.Lon();
}

bool Geometry::Limits::IsPositionWithinLimits(const LatLon& pos) const {
	if (!valid) return true; // If no limit or not valid limit accept it
	assert(pos.IsValid());
	if (pos.Lat() > topLeft.Lat() || pos.Lat() < bottomRight.Lat()) return false;
	if (acrossAntiGreenwich) return pos.Lon() >= 0 ? pos.Lon() <= topLeft.Lon() : pos.Lon() <= bottomRight.Lon();
	else return pos.Lon() >= topLeft.Lon() && pos.Lon() <= bottomRight.Lon();
}

bool Geometry::Limits::IsPositionWithinLimits(const double& lat, const double& lon) const {
	if (!valid) return true; // If no limit or not valid limit accept it
	assert(LatLon::IsValidLat(lat) && LatLon::IsValidLon(lon));
	if (lat > topLeft.Lat() || lat < bottomRight.Lat()) return false;
	if (acrossAntiGreenwich) return lon >= 0 ? lon <= topLeft.Lon() : lon <= bottomRight.Lon();
	else return lon >= topLeft.Lon() && lon <= bottomRight.Lon();
}

double Geometry::AbsAngle(const double& angle) { //to put angle in the range between 0 and 2PI
	assert(!std::isinf(angle) && !std::isnan(angle));
	double absangle = std::fmod(angle, TWO_PI);
	if (absangle < 0) absangle += TWO_PI;
	assert(absangle >= 0 && absangle <= TWO_PI);
	return absangle;
}

double Geometry::DeltaAngle(const double angle, const double reference) {
	assert(angle >= 0 && angle <= TWO_PI);
	assert(reference >= 0 && reference <= TWO_PI);
	double delta = angle - reference;
	if (delta > PI) delta -= TWO_PI;
	else if (delta < -PI) delta += TWO_PI;
	assert(delta >= -PI && delta <= PI);
	return delta;
}

double Geometry::AnglePi2Pi(const double& angle) { //to put angle in the range between -PI and PI
	assert(!std::isinf(angle) && !std::isnan(angle));
	double anglePi2Pi = std::fmod(angle, TWO_PI);
	if(anglePi2Pi > PI) anglePi2Pi -= TWO_PI;
	else if(anglePi2Pi < -PI) anglePi2Pi += TWO_PI;
	assert(anglePi2Pi >= -PI && anglePi2Pi <= PI);
	return anglePi2Pi;
}

// This requires pre-computation of distance
double Geometry::CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& d) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	if (lat2 == PI_2) return TWO_PI; //we are going to N pole
	if (lat2 == -PI_2) return PI; //we are going to S pole
	const double dlon = lon2 - lon1;
	if (std::fabs(dlon) <= TOL) return lat1 > lat2 ? PI : TWO_PI; // if (lon1 == lon2) we are going to E or W
	return AbsAngle(sin(dlon) < 0 ? acos((sin(lat2) - sin(lat1)*cos(d)) / (sin(d)*cos(lat1))) : TWO_PI - acos((sin(lat2) - sin(lat1)*cos(d)) / (sin(d)*cos(lat1))));
}

// This not require pre-computation of distance
double Geometry::CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	if (lat2 == PI_2) return TWO_PI; //we are going to N pole
	if (lat2 == -PI_2) return PI; //we are going to S pole
	const double dlon = lon1 - lon2;
	if (std::fabs(dlon) <= TOL) return lat1 > lat2 ? PI : TWO_PI; // if (lon1 == lon2) we are going to E or W
	const double coslat2 = cos(lat2);
	return AbsAngle(atan2(sin(dlon)*coslat2, cos(lat1)*sin(lat2) - sin(lat1)*coslat2*cos(dlon)));
}

double Geometry::CalcAngularDist(const double& lat1, const double& lon1, const double& lat2, const double& lon2) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	return 2.0 * asin(sqrt(pow(sin((lat1 - lat2) / 2), 2) + cos(lat1) * cos(lat2) * pow(sin((lon1 - lon2) / 2), 2)));
}

void Geometry::CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst, double& lat, double& lon) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(dir >= 0 && dir <= 2 * TWO_PI);
	const double sinlat1 = sin(lat1);
	const double cosdst = cos(dst);
	const double coslat1 = cos(lat1);
	const double sindst = sin(dst);
	lat = asin(sinlat1 * cosdst + coslat1 * sindst * cos(dir));

	//a way to have lon always between PI and -PI but
	//lon = std::fmod(lon1 - atan2(sin(dir) * sindst * coslat1, cosdst - sinlat1 * sin(lat)) + PI, TWO_PI) - PI;
	lon = AnglePi2Pi(lon1 - atan2(sin(dir) * sindst * coslat1, cosdst - sinlat1 * sin(lat)));

	assert(lat >= -PI_2 && lat <= PI_2);
	assert(lon >= -PI && lon <= PI);
}

Geometry::LatLon Geometry::CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst) {
	double lat=0, lon=0;
	CalcRadialPoint(lat1, lon1, dir, dst, lat, lon);
	return LatLon::CreateFromRadiants(lat,lon);
}

double Geometry::FindStep(const double& radius, const double& angle) {
	assert(angle >= 0 && angle <= TWO_PI);
	assert(radius >= 0 && radius <= PI_2);
	static const double smallRadius = 3 * NM2RAD; // 3 NM, radius under it the number of points will be decreased
	static const double m = 300 / smallRadius; // coeffcient to decrease points for small circles, 300 is default number of points for circles bigger than 3 NM
	const int steps((int)(radius < smallRadius ?
			(angle * (m * radius + 8)) / TWO_PI: // 8 is the minimum number of points for circles with radius -> 0
			(angle * radius) / resolution));
	return angle / steps;
}

bool Geometry::CalcBisector(const double& latA, const double& lonA, const double& latB, const double& lonB, const double& latC, const double& lonC, double& bisector) {
	const double crsBA = CalcGreatCircleCourse(latB, lonB, latA, lonA);
	const double crsBC = CalcGreatCircleCourse(latB, lonB, latC, lonC);
	double diff = DeltaAngle(crsBC, crsBA); //angle between the directions
	if (diff>0) { //positive difference: the internal angle in on the left
		diff = AbsAngle(diff);
		bisector = AbsAngle(crsBA + diff / 2);
		return false; // counter clockwise turn
	} else { //negative difference: the internal angle is on the right
		diff = AbsAngle(-diff);
		bisector = AbsAngle(crsBA - diff / 2);
		return true; // clockwise turn
	}
}

void Geometry::CalcSphericalTriangle(const double& a, const double& beta, const double& gamma, double& alpha, double& b, double& c) {
	assert(a >= 0 && a <= TWO_PI);
	assert(beta >= 0 && beta <= PI);
	assert(gamma >= 0 && gamma <= PI);
	alpha = acos(sin(beta)*sin(gamma)*cos(a) - cos(beta)*cos(gamma));
	b = atan2(sin(a)*sin(beta)*sin(gamma), cos(beta) + cos(gamma)*cos(alpha));
	c = atan2(sin(a)*sin(beta)*sin(gamma), cos(gamma) + cos(alpha)*cos(beta));
	assert(alpha >= 0 && alpha <= PI);
	assert(b >= 0 && b <= TWO_PI);
	assert(c >= 0 && c <= TWO_PI);
}

bool Geometry::CalcRadialIntersection(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& crs13, const double& crs23, double& lat3, double& lon3, double& dst13, double& dst23) {
	if (std::fabs(DeltaAngle(crs13, crs23)) <= TOL) return false;
	const double dst12 = CalcAngularDist(lat1, lon1, lat2, lon2);
	const double crs12 = CalcGreatCircleCourse(lat1, lon1, lat2, lon2, dst12);
	const double crs21 = CalcGreatCircleCourse(lat2, lon2, lat1, lon1, dst12);

	//Angle PI -PI ?
	//double ang1 = std::fmod(crs13 - crs12 + PI, TWO_PI) - PI;
	//double ang2 = std::fmod(crs21 - crs23 + PI, TWO_PI) - PI;
	double ang1 = AnglePi2Pi(crs13 - crs12);
	double ang2 = AnglePi2Pi(crs21 - crs23);

	if ((sin(ang1) == 0 && sin(ang2) == 0) || sin(ang1) * sin(ang2) < 0) return false;  //infinity of intersections or intersection ambiguous
	ang1 = std::fabs(ang1);
	if (ang1 <= TOL || ang1 >= PI - TOL) return false;
	ang2 = std::fabs(ang2);
	if (ang2 <= TOL || ang2 >= PI - TOL) return false;
	double ang3 = 0;
	CalcSphericalTriangle(dst12, ang1, ang2, ang3, dst13, dst23);
	CalcRadialPoint(lat1, lon1, crs13, dst13, lat3, lon3);
	return true;
}

bool Geometry::ArePointsOnArc(const LatLon& A, const LatLon& B, const LatLon& C, double& latc, double& lonc, double& radius, bool& clockwise) {
	static const double maxDst = (5000 / NM2M) * NM2RAD; // Do not process segments longer than 5 Km
	const double latA = A.LatRad(); //TODO: maybe avoid to recalculate always
	const double lonA = A.LonRad();
	const double latB = B.LatRad();
	const double lonB = B.LonRad();
	const double latC = C.LatRad();
	const double lonC = C.LonRad();

	// Check distances
	const double dstAB = CalcAngularDist(latA, lonA, latB, lonB);
	if (dstAB > maxDst) return false;
	const double dstBC = CalcAngularDist(latB, lonB, latC, lonC);
	if (dstBC > maxDst) return false;
	
	// Calculate courses
	const double crsAB = CalcGreatCircleCourse(latA, lonA, latB, lonB, dstAB);
	const double crsBA = CalcGreatCircleCourse(latB, lonB, latA, lonA, dstAB);
	const double crsBC = CalcGreatCircleCourse(latB, lonB, latC, lonC, dstBC);
	
	// Calculate course difference between two segments
	const double delta = DeltaAngle(crsBC, crsBA);
	const double absDelta = std::fabs(delta);
	if (absDelta <= TOL || absDelta >= PI - TOL) return false; // aligned points so not on an arc of circle!
	clockwise = delta < 0; // negative difference: internal angle is on the right so turning clockwise
	
	// Calculate middle point 1 on segment AB and it's orthogonal course to the center
	const double dstAB2 = dstAB / 2;
	double lat1 = 0, lon1 = 0;
	CalcRadialPoint(latA, lonA, crsAB, dstAB2, lat1, lon1);
	const double crs1A = CalcGreatCircleCourse(lat1, lon1, latA, lonA, dstAB2);
	const double crs1c = AbsAngle(clockwise ? crs1A - PI_2 : crs1A + PI_2);
	
	// Calculate middle point 2 on segment BC and it's orthogonal course to the center
	const double dstBC2 = dstBC / 2;
	double lat2 = 0, lon2 = 0;
	CalcRadialPoint(latB, lonB, crsBC, dstBC2, lat2, lon2);
	const double crs2B = CalcGreatCircleCourse(lat2, lon2, latB, lonB, dstBC2);
	const double crs2c = AbsAngle(clockwise ? crs2B - PI_2 : crs2B + PI_2);

	// Intersect the two orthogonal courses to find the center
	double radius2;
	if (!CalcRadialIntersection(lat1, lon1, lat2, lon2, crs1c, crs2c, latc, lonc, radius, radius2)) return false;

	// Make sure that the radius is not something incredibly big
	if (radius > PI_2) return false;

	// Compare the two radius they must be similar
	const double diff = std::fabs(radius - radius2);
	const double limit = radius / 8;
	return diff < limit;
}

Geometry::LatLon Geometry::AveragePoints(const std::vector<std::pair<const double, const double>>& centerPoints) {
	assert(!centerPoints.empty());
	double latc = 0;
	double lonc = 0;
	for (const std::pair<const double, const double>& p : centerPoints) {
		latc += p.first;
		lonc += p.second;
	}
	const int num = (int)centerPoints.size();
	latc /= num;
	lonc /= num;
	assert(latc >= -PI_2 && latc <= PI_2);
	assert(lonc >= -PI && lonc <= PI);
	return LatLon::CreateFromRadiants(latc, lonc);
}

double Geometry::AverageRadius(const Geometry::LatLon& center, const std::vector<Geometry::LatLon*>& circlePoints) {
	assert(!circlePoints.empty());
	double latc = center.LatRad();
	double lonc = center.LonRad();
	double radius = 0;
	for (const Geometry::LatLon* p : circlePoints) radius += CalcAngularDist(latc, lonc, p->LatRad(), p->LonRad());
	return (radius / circlePoints.size()) * 1.000675456; // corrected average
}

// Convert a distance from rad to NM rounding it: (if it is 4.99663 NM make it 5 NM)
double Geometry::RoundDistanceInNM(const double radiusRad) {
	assert(!std::isnan(radiusRad));
	assert(radiusRad > 0);
	double radius(radiusRad * RAD2NM); // [NM]
	const double radius100(radius * 100); // [NM * 100] approximate on cents of NM
	const double nearestInt(std::round(radius100)); // [NM * 100]
	const double diff(std::fabs(nearestInt - radius100)); // [NM * 100]
	if (nearestInt != 0 && diff > 0 && diff < 0.25) radius = nearestInt / 100; //0.0025 NM = 4.63 m
	return radius;
}

bool Geometry::IsInt(const double& number, int& intVal) {
	intVal = (int)std::round(number);
	return std::fabs(number-intVal) < TOL;
}

bool Geometry::CalcAirfieldPolygon(const double lat, const double lon, const int length, const int dir, std::vector<LatLon>& polygon) {
	static const double thrtyMeters = 30.0 * M2RAD;
	assert(polygon.empty());
	assert(length > 0);
	assert(dir >= 0 && dir <= 360);
	const double clat = lat * DEG2RAD;
	const double clon = -lon * DEG2RAD;
	const double fdir = dir * DEG2RAD;
	const double bdir = AbsAngle(fdir + PI);
	const double hlen = (length / 2) * M2RAD;

	double llat, llon;
	CalcRadialPoint(clat, clon, AbsAngle(fdir - PI_2), thrtyMeters, llat, llon);

	double rlat, rlon;
	CalcRadialPoint(clat, clon, AbsAngle(fdir + PI_2), thrtyMeters, rlat, rlon);

	double plat, plon;
	CalcRadialPoint(llat, llon, fdir, hlen, plat, plon);
	polygon.push_back(LatLon::CreateFromRadiants(plat, plon));

	CalcRadialPoint(rlat, rlon, fdir, hlen, plat, plon);
	polygon.push_back(LatLon::CreateFromRadiants(plat, plon));

	CalcRadialPoint(rlat, rlon, bdir, hlen, plat, plon);
	polygon.push_back(LatLon::CreateFromRadiants(plat, plon));

	CalcRadialPoint(llat, llon, bdir, hlen, plat, plon);
	polygon.push_back(LatLon::CreateFromRadiants(plat, plon));

	assert(polygon.size() == 4);
	return true;
}

bool Point::Discretize(std::vector<LatLon>& output) const {
	output.push_back(point); // Here it's easy :)
	return true;
}

void Point::WriteOpenAirGeometry(OpenAir& openAir) const {
	openAir.WritePoint(*this);
}

Sector::Sector(const LatLon& center, const double radiusNM, const double dir1, const double dir2, const bool isClockwise)
	: Geometry(center)
	, clockwise(isClockwise)
	, latc(center.LatRad())
	, lonc(center.LonRad())
	, angleStart(AbsAngle(dir1 * DEG2RAD))
	, angleEnd(AbsAngle(dir2 * DEG2RAD))
	, radius(radiusNM * NM2RAD)
	, A(CalcRadialPoint(latc, lonc, angleStart, radius))
	, B(CalcRadialPoint(latc, lonc, angleEnd, radius)) {
	assert(radius > 0 && radius < PI_2);
}

Sector::Sector(const LatLon& center, const LatLon& pointStart, const LatLon& pointEnd, const bool isClockwise)
	: Geometry(center)
	, clockwise(isClockwise)
	, latc(center.LatRad())
	, lonc(center.LonRad())
	, A(pointStart)
	, B(pointEnd) {
	const double lat1r = pointStart.LatRad();
	const double lon1r = pointStart.LonRad();
	const double lat2r = pointEnd.LatRad();
	const double lon2r = pointEnd.LonRad();
	radius = CalcAngularDist(latc, lonc, lat1r, lon1r);
	assert(radius > 0 && radius < PI_2);
	assert(std::fabs((radius * RAD2NM) - (CalcAngularDist(latc, lonc, lat2r, lon2r) * RAD2NM)) < 0.2);
	angleStart = CalcGreatCircleCourse(latc, lonc, lat1r, lon1r);
	angleEnd = CalcGreatCircleCourse(latc, lonc, lat2r, lon2r);
}

bool Sector::Discretize(std::vector<LatLon>& output) const {
	if (clockwise) {
		double e = angleStart <= angleEnd ? angleEnd : angleEnd + TWO_PI;
		const double step = FindStep(radius, AbsAngle(e - angleStart));
		assert(angleStart <= e);
		for (double a = angleStart; a < e; a += step) output.push_back(CalcRadialPoint(latc, lonc, a, radius));
		output.push_back(CalcRadialPoint(latc, lonc, e, radius)); // Add the exact last point
	} else {
		const double s = angleStart >= angleEnd ? angleStart : angleStart + TWO_PI;
		const double step = FindStep(radius, AbsAngle(s - angleEnd));
		assert(s >= angleEnd);
		for (double a = s; a > angleEnd; a -= step) output.push_back(CalcRadialPoint(latc, lonc, a, radius));
		output.push_back(CalcRadialPoint(latc, lonc, angleEnd, radius)); // Add the exact last point
	}
	return true;
}

void Sector::WriteOpenAirGeometry(OpenAir& openAir) const {
	openAir.WriteSector(*this);
}

Circle::Circle(const LatLon& center, const double& radiusNM)
	: Geometry(center)
	, radius(radiusNM * NM2RAD)
	, latc(center.LatRad())
	, lonc(center.LonRad()) {
	assert(radius > 0 && radius < PI_2);
}

bool Circle::Discretize(std::vector<LatLon>& output) const {
	const double step = FindStep(radius, TWO_PI);
	for (double a = 0; a < TWO_PI; a += step) output.push_back(CalcRadialPoint(latc, lonc, a, radius));
	return true;
}

void Circle::WriteOpenAirGeometry(OpenAir& openAir) const {
	openAir.WriteCircle(*this);
}

/* Airway not supported yet
bool AirwayPoint::Discretize(std::vector<LatLon>& output) const {
	return false;
}
*/
