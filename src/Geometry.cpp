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

#include "Geometry.h"
#include "OpenAir.h"
#include <cmath>
#include <cassert>

const int Geometry::LatLon::UNDEF_LAT = -91;
const int Geometry::LatLon::UNDEF_LON = -181;
const double Geometry::LatLon::SIXTYTH = 1.0 / 60;

const double Geometry::PI = 3.1415926535897932384626433832795;
const double Geometry::TWO_PI = PI * 2;
const double Geometry::PI_2 = PI / 2;
const double Geometry::DEG2RAD = PI / 180;
const double Geometry::RAD2DEG = 180.0 / PI;
const double Geometry::NM2RAD = PI / (180 * 60);
const double Geometry::RAD2NM = (180.0 * 60.0) / PI;
const double Geometry::NM2M = 1852.0;

double Geometry::resolution = 0.3 * NM2RAD; // 0.3 NM = 555.6 m

void Geometry::LatLon::convertDec2DegMin(const double& dec, int& deg, double& min) {
	double decimal = std::fabs(dec);
	deg=(int)floor(decimal);
	min = (decimal-deg)/SIXTYTH;
}

double Geometry::AbsAngle(const double& angle) { //to put angle in the range between 0 and 2PI
	assert(!isinf(angle) && !isnan(angle));
	double absangle = std::fmod(angle, TWO_PI);
	if (absangle < 0) absangle += TWO_PI;
	assert(absangle >= 0 && absangle <= TWO_PI);
	return absangle;
}

double Geometry::AnglePi2Pi(const double& angle) { //to put angle in the range between -PI and PI
	assert(!isinf(angle) && !isnan(angle));
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
	if (std::fabs(lon1 - lon2) <= 1e-10) return lat1>lat2 ? PI : TWO_PI; // if (lon1 == lon2) we are going to E or W
	return AbsAngle(sin(lon2 - lon1) < 0 ? acos((sin(lat2) - sin(lat1)*cos(d)) / (sin(d)*cos(lat1))) : TWO_PI - acos((sin(lat2) - sin(lat1)*cos(d)) / (sin(d)*cos(lat1))));
}

// This not require pre-computation of distance
double Geometry::CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	if (lat2 == PI_2) return TWO_PI; //we are going to N pole
	if (lat2 == -PI_2) return PI; //we are going to S pole
	if (lon1 == lon2) return lat1>lat2 ? PI : TWO_PI; //we are going to E or W
	const double dlon = lon1 - lon2;
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
	assert(dir > -PI && dir <= 2 * TWO_PI);
	const double sinlat1 = sin(lat1);
	const double cosdst = cos(dst);
	const double coslat1 = cos(lat1);
	const double sindst = sin(dst);
	lat = asin(sinlat1 * cosdst + coslat1 * sindst * cos(dir));

	//TODO: check this: a way to have lon alwyas between PI and -PI but not always works
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
	int steps;
	if (radius < smallRadius) steps = (int)((angle * (m * radius + 8)) / TWO_PI); // 8 is the minimum number of points for circles with radius -> 0
	else steps = (int)((angle * radius) / resolution);
	return angle / steps;
}

double Geometry::DeltaAngle(const double angle, const double reference)
{
	assert(angle >= 0 && angle <= TWO_PI);
	assert(reference >= 0 && reference <= TWO_PI);
	double delta = angle - reference;
	if (delta > PI) delta -= TWO_PI;
	else if (delta < -PI) delta += TWO_PI;
	assert(delta >= -TWO_PI && delta <= TWO_PI);
	return delta;
}

bool Geometry::CalcBisector(const double& latA, const double& lonA, const double& latB, const double& lonB, const double& latC, const double& lonC, double& bisector) {
	const double courseBA = CalcGreatCircleCourse(latB, lonB, latA, lonA); //TODO: this can be done only one time
	const double courseBC = CalcGreatCircleCourse(latB, lonB, latC, lonC);

	double diff = DeltaAngle(courseBC, courseBA); //angle between the directions

	if (diff>0) { //positive difference: the internal angle in on the left
		diff = AbsAngle(diff);
		bisector = AbsAngle(courseBA + diff / 2);
		return false; // counter clockwise turn
	}
	else { //negative difference: the internal angle is on the right
		diff = AbsAngle(-diff);
		bisector = AbsAngle(courseBA - diff / 2);
		return true; // clockwise turn
	}
}

void Geometry::CalcSphericalTriangle(const double& a, const double& beta, const double& gamma, double& alpha, double& b, double& c)
{
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
	const double dst12 = CalcAngularDist(lat1, lon1, lat2, lon2); //TODO: this can be done only one time
	const double crs12 = CalcGreatCircleCourse(lat1, lon1, lat2, lon2, dst12); //TODO: this can be done only one time
	const double crs21 = CalcGreatCircleCourse(lat2, lon2, lat1, lon1, dst12); //TODO: this can be done only one time

	//TODO: angle PI -PI
	//double ang1 = std::fmod(crs13 - crs12 + PI, TWO_PI) - PI;
	//double ang2 = std::fmod(crs21 - crs23 + PI, TWO_PI) - PI;
	//const double ang1 = AbsAngle(crs13 - crs12);
	//const double ang2 = AbsAngle(crs21 - crs23);
	//double ang1 = DeltaAngle(crs13, crs12);
	//double ang2 = DeltaAngle(crs21, crs23);
	double ang1 = AnglePi2Pi(crs13 - crs12);
	double ang2 = AnglePi2Pi(crs21 - crs23);


	if ((sin(ang1) == 0 && sin(ang2) == 0) || sin(ang1) * sin(ang2) < 0) return false;  //infinity of intersections or intersection ambiguous
	ang1 = std::fabs(ang1);
	ang2 = std::fabs(ang2);
	double ang3 = 0;
	CalcSphericalTriangle(dst12, ang1, ang2, ang3, dst13, dst23);
	CalcRadialPoint(lat1, lon1, crs13, dst13, lat3, lon3);
	return true;
}

bool Geometry::ArePointsOnArc(const LatLon& A, const LatLon& B, const LatLon& C, double& latc, double& lonc, double& radius, bool& clockwise) {
	static const double minDst = (5000 / NM2M) * NM2RAD; // Do not process segments longer than 5 Km
	const double latA = A.LatRad(); //TODO: avoid to recalculate always
	const double lonA = A.LonRad();
	const double latB = B.LatRad();
	const double lonB = B.LonRad();
	const double latC = C.LatRad();
	const double lonC = C.LonRad();

	// Check distances
	const double dstAB = CalcAngularDist(latA, lonA, latB, lonB);
	if (dstAB > minDst) return false;
	const double dstBC = CalcAngularDist(latB, lonB, latC, lonC);
	if (dstBC > minDst) return false;
	
	// Calculate courses
	const double crsAB = CalcGreatCircleCourse(latA, lonA, latB, lonB, dstAB);
	const double crsBA = CalcGreatCircleCourse(latB, lonB, latA, lonA, dstAB);
	const double crsBC = CalcGreatCircleCourse(latB, lonB, latC, lonC, dstBC);
	
	// Calculate course difference between two segments
	const double delta = DeltaAngle(crsBC, crsBA);
	if (delta == 0) return false; // aligned points so not on an arc of circle!
	clockwise = delta < 0; // negative difference: internal angle is on the right so turning clockwise
	
	// Calculate middle point 1 on segment AB and it's orthogonal course to the center
	const double dstAB2 = dstAB / 2;
	double lat1=0, lon1=0;
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
	CalcRadialIntersection(lat1, lon1, lat2, lon2, crs1c, crs2c, latc, lonc, radius, radius2);

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
	const int num = centerPoints.size();
	latc /= num;
	lonc /= num;
	return LatLon::CreateFromRadiants(latc, lonc);
}

double Geometry::AverageRadius(const Geometry::LatLon& center, const std::vector<Geometry::LatLon*>& circlePoints) {
	assert(!circlePoints.empty());
	double latc = center.LatRad();
	double lonc = center.LonRad();
	double radius = 0;
	for (const Geometry::LatLon* p : circlePoints) radius += CalcAngularDist(latc, lonc, p->LatRad(), p->LonRad());
	return radius / circlePoints.size();
}

bool Point::Discretize(std::vector<LatLon>& output) const
{
	output.push_back(point); // Here it's easy :)
	return true;
}

void Point::WriteOpenAirGeometry(OpenAir* openAir) const
{
	openAir->WritePoint(this);
}

Sector::Sector(const LatLon& center, const double& radiusNM, const double& dir1, const double& dir2, const bool& isClockwise)
	: Geometry(center)
	, clockwise(isClockwise)
	, latc(center.LatRad())
	, lonc(center.LonRad())
	, angleStart(dir1 * DEG2RAD)
	, angleEnd(dir2 * DEG2RAD)
	, radius(radiusNM * NM2RAD)
	, A(CalcRadialPoint(latc, lonc, angleStart, radius))
	, B(CalcRadialPoint(latc, lonc, angleEnd, radius)) {
}

Sector::Sector(const LatLon& center, const LatLon& pointStart, const LatLon& pointEnd, const bool& isClockwise)
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

	assert(radius > 0);
	assert(std::fabs((radius * RAD2NM) - (CalcAngularDist(latc, lonc, lat2r, lon2r) * RAD2NM)) < 0.2);

	angleStart = CalcGreatCircleCourse(latc, lonc, lat1r, lon1r);
	angleEnd = CalcGreatCircleCourse(latc, lonc, lat2r, lon2r);
}

bool Sector::Discretize(std::vector<LatLon>& output) const
{
	if (clockwise) {
		double e = angleStart <= angleEnd ? angleEnd : angleEnd + TWO_PI;
		const double step = FindStep(radius, AbsAngle(e - angleStart));
		assert(angleStart <= e);
		for (double a = angleStart; a < e; a += step)
			output.push_back(CalcRadialPoint(latc, lonc, a, radius));
	} else {
		const double s = angleStart >= angleEnd ? angleStart : angleStart + TWO_PI;
		const double step = FindStep(radius, AbsAngle(s - angleEnd));
		assert(s >= angleEnd);
		for (double a = s; a > angleEnd; a -= step)
			output.push_back(CalcRadialPoint(latc, lonc, a, radius));
	}
	return true;
}

void Sector::WriteOpenAirGeometry(OpenAir* openAir) const
{
	openAir->WriteSector(this);
}

Circle::Circle(const LatLon& center, const double& radiusNM)
	: Geometry(center)
	, radius(radiusNM * NM2RAD)
	, latc(center.LatRad())
	, lonc(center.LonRad()) {
}

bool Circle::Discretize(std::vector<LatLon>& output) const
{
	const double step = FindStep(radius, TWO_PI);
	for (double a = 0; a < TWO_PI; a += step) output.push_back(CalcRadialPoint(latc, lonc, a, radius));
	return true;
}

void Circle::WriteOpenAirGeometry(OpenAir* openAir) const
{
	openAir->WriteCircle(this);
}

/* Airway not supported yet
bool AirwayPoint::Discretize(std::vector<LatLon>& output) const
{
	// TODO...
	return false;
}
*/
