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
#include "OpenAir.h"
#include <string>
#include <cmath>
#include <cassert>

const int LatLon::UNDEF_LAT = -91;
const int LatLon::UNDEF_LON = -181;
const double LatLon::SIXTYTH = 1.0 / 60;

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

void LatLon::convertDec2DegMin(const double& dec, int& deg, double& min) {
	double decimal = fabs(dec);
	deg=(int)floor(decimal);
	min = (decimal-deg)/SIXTYTH;
}

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

Airspace::~Airspace() {
	for (const Geometry* g : geometries) delete g;
	geometries.clear();
}

void Airspace::Clear() {
	type = UNKNOWN;
	name.clear();
	for (const Geometry* g : geometries) delete g;
	geometries.clear();
	points.clear();
}

void Airspace::AddPoint(const double& lat, const double& lon) {
	LatLon point(lat, lon);
	geometries.push_back(new Point(point));
	points.push_back(point);
}

void Airspace::AddGeometry(const Geometry* geometry) {
	geometries.push_back(geometry);
	geometry->Discretize(points);
}

bool Airspace::Undiscretize()
{
	/////TESTING////////////////
	bool discretize = false;
	bool forceClean = true;
	if (discretize || forceClean) {
		for (const Geometry* g : geometries) delete g;
		geometries.clear();
	}
	////END TESTING/////////////////////////
	
	if (!geometries.empty()) return true;
	if (points.empty()) return false;

	/////TESTING////////////////////////////////////////////////////////////////
	//TODO: for now we just print down all the points, but would be nice to "undiscretize" back to circles and arcs
	// Brutally insert all the points
	if (discretize) {
		for (const LatLon& point : points) {
			geometries.push_back(new Point(point));
		}
		return true;
	}
	/////END TESTING///////////////////////////////////////////////////////////////////

	//TODO: to be done!
	assert(points.size() >= 4);
	unsigned int steps = points.size() - 3;

	LatLon startPoint, endPoint, centerPoint;
	double currentRadius=0;
	bool alreadyOnArc = false;
	bool counterClockwise;
	for (unsigned int i = 0; i < steps; i++) {
		double latc=LatLon::UNDEF_LAT, lonc=LatLon::UNDEF_LON, radius=0;
		bool clockwise;
	
		if (Geometry::ArePointsOnArc(points.at(i), points.at(i + 1), points.at(i + 2), latc, lonc, radius, clockwise)) {
			endPoint = points.at(i + 2);
			if (alreadyOnArc) { // the arc continues
				//TODO: Make a proper check if the center is the same!!!
				
				//const double deltaRadius = std::fabs(radius - currentRadius) * Geometry::RAD2NM;
				//assert(deltaRadius < 0.0267); // ASSERTION FAILED!
			} else { // New arc
				startPoint = points.at(i);
				currentRadius = radius;
				centerPoint = LatLon(latc * Geometry::RAD2DEG, -lonc * Geometry::RAD2DEG);
				counterClockwise = clockwise;
				alreadyOnArc = true;
			
			}
		} else {
			if (alreadyOnArc) {
				alreadyOnArc = false;
				geometries.push_back(new Sector(centerPoint.Lat(), centerPoint.Lon(), startPoint.Lat(), startPoint.Lon(), endPoint.Lat(), endPoint.Lon(), counterClockwise));
			} else
				geometries.push_back(new Point(points.at(i)));
		}
	}
	if (alreadyOnArc) {
		assert(currentRadius > 0);
		geometries.push_back(new Circle(centerPoint.Lat(), centerPoint.Lon(), currentRadius * Geometry::RAD2NM));
	}
	return true;
}

double Geometry::AbsAngle(const double& angle) { //to put angle in the range between 0 and 2PI
	if (isinf(angle)) return TWO_PI;
	double absangle = std::fmod(angle, TWO_PI);
	if (absangle<0) absangle += TWO_PI;
	return absangle;
}

// This requires pre-computation of distance
double Geometry::CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& d) {	
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	//assert(CalcAngularDist(lat1, lon1, lat2, lon2) == d);
	if (lat2 == PI_2) return TWO_PI; //we are going to N pole
	if (lat2 == -PI_2) return PI; //we are going to S pole
	if (lon1 == lon2) return lat1>lat2 ? PI : TWO_PI; //we are going to E or W
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
	return AbsAngle(atan2(sin(lon1 - lon2)*cos(lat2), cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lon1 - lon2)));
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
	lat = asin(sin(lat1) * cos(dst) + cos(lat1) * sin(dst) * cos(dir));
	lon = std::fmod(lon1 - atan2(sin(dir) * sin(dst) * cos(lat1), cos(dst) - sin(lat1) * sin(lat)) + PI, TWO_PI) - PI;
}

LatLon Geometry::CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst) {
	double lat=0, lon=0;
	CalcRadialPoint(lat1, lon1, dir, dst, lat, lon);
	return LatLon(lat * RAD2DEG, -lon * RAD2DEG);
}

double Geometry::FindStep(const double& radius, const double& angle) {
	assert(angle >= 0 && angle <= TWO_PI);
	assert(radius >= 0);
	static const double smallRadius = 3 * NM2RAD; // 3 NM, radius under it the number of points will be decreased
	static const double m = 300 / smallRadius; // coeffcient to decrease points for small circles, 300 is default number of points for circles bigger than 3 NM
	int steps;
	if (radius < smallRadius) {
		steps = (int)((angle * (m * radius + 8)) / TWO_PI); // 8 is the minimum number of points for circles with radius -> 0
	}
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
	return delta;
}

bool Geometry::CalcBisector(const double& latA, const double& lonA, const double& latB, const double& lonB, const double& latC, const double& lonC, double& bisector) {
	const double courseBA = CalcGreatCircleCourse(latB, lonB, latA, lonA); //TODO: this can be done only one time
	const double courseBC = CalcGreatCircleCourse(latB, lonB, latC, lonC);

	//TEST
	double courseBAdeg = courseBA * RAD2DEG;
	double courseBCdeg = courseBC * RAD2DEG;
	courseBAdeg++;
	courseBCdeg++;

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

//TODO: verify
void Geometry::CalcSphericalTriangle(const double& a, const double& beta, const double& gamma, double& alpha, double& b, double& c)
{
	assert(a >= 0 && a <= TWO_PI);
	assert(beta >= 0 && gamma <= TWO_PI);
	assert(gamma >= 0 && gamma <= TWO_PI);
	alpha = acos(sin(beta)*sin(gamma)*cos(a) - cos(beta)*cos(gamma));
	b = atan2(sin(a)*sin(beta)*sin(gamma), cos(beta) + cos(gamma)*cos(alpha));
	c = atan2(sin(a)*sin(beta)*sin(gamma), cos(gamma) + cos(alpha)*cos(beta));
}


bool Geometry::CalcRadialIntersection(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& crs13, const double& crs23, double& lat3, double& lon3, double& dst13, double& dst23) {
	const double dst12 = CalcAngularDist(lat1, lon1, lat2, lon2); //TODO: this can be done only one time
	const double crs12 = CalcGreatCircleCourse(lat1, lon1, lat2, lon2, dst12); //TODO: this can be done only one time
	const double crs21 = CalcGreatCircleCourse(lat2, lon2, lat1, lon1, dst12); //TODO: this can be done only one time

	//TEST:////////////////////////////
	double dstNM = dst12 * RAD2NM;
	double crs12deg = crs12 * RAD2DEG;
	double crs21deg = crs21 * RAD2DEG;
	dstNM++;
	crs12deg++;
	crs21deg++;
	//////////////////////////////////
	
	double ang1 = std::fmod(crs13 - crs12 + PI, TWO_PI) - PI;
	double ang2 = std::fmod(crs21 - crs23 + PI, TWO_PI) - PI;

	//TEST://////////////////////////////
	double a1deg = ang1 * RAD2DEG;
	double a2deg = ang2 * RAD2DEG;
	a1deg++;
	a2deg++;
	////////////////////////////////////


	if ( (sin(ang1) == 0 && sin(ang2) == 0) || sin(ang1)*sin(ang2) < 0) return false;  //infinity of intersections or intersection ambiguous
	
	ang1 = std::fabs(ang1);
	ang2 = std::fabs(ang2);

	double ang3 = 0;
		
	CalcSphericalTriangle(dst12, ang1, ang2, ang3, dst23, dst13);

	assert(dst13 > 0); //FIXME: Assertion failed

	CalcRadialPoint(lat1, lon1, crs13, dst13, lat3, lon3);

	return true;
}

bool Geometry::ArePointsOnArc(const LatLon& A, const LatLon& B, const LatLon& C, double& latc, double& lonc, double& radius, bool& clockwise) {
	const double latA = A.Lat() * DEG2RAD; //TODO: avoid to recalculate alaways
	const double lonA = - A.Lon() * DEG2RAD;
	const double latB = B.Lat() * DEG2RAD;
	const double lonB = - B.Lon() * DEG2RAD;
	const double latC = C.Lat() * DEG2RAD;
	const double lonC = - C.Lon() * DEG2RAD;

	const double dstAB = CalcAngularDist(latA, lonA, latB, lonB);
	const double crsAB = CalcGreatCircleCourse(latA, lonA, latB, lonB, dstAB);
	const double crsBA = CalcGreatCircleCourse(latB, lonB, latA, lonA, dstAB);

	const double dstBC = CalcAngularDist(latB, lonB, latC, lonC);
	const double crsBC = CalcGreatCircleCourse(latB, lonB, latC, lonC, dstBC);
	
	const double delta = DeltaAngle(crsBC, crsBA);
	if (delta == 0) return false; // aligned points  so not on an arc of circle!
	
	clockwise = delta < 0; // negative difference: internal angle is on the right so turning clockwise

	const double dstAB2 = dstAB / 2;
	double lat1=0, lon1=0;
	CalcRadialPoint(latA, lonA, crsAB, dstAB2, lat1, lon1);
	const double crs1A = CalcGreatCircleCourse(lat1, lon1, latA, lonA, dstAB2);
	const double crs1c = AbsAngle(clockwise ? crs1A - PI_2 : crs1A + PI_2);

	const double dstBC2 = dstBC / 2;
	double lat2 = 0, lon2 = 0;
	CalcRadialPoint(latB, lonB, crsBC, dstBC2, lat2, lon2);
	const double crs2B = CalcGreatCircleCourse(lat2, lon2, latB, lonB, dstBC2);
	const double crs2c = AbsAngle(clockwise ? crs2B - PI_2 : crs2B + PI_2);

	double radius2;
	CalcRadialIntersection(lat1, lon1, lat2, lon2, crs1c, crs2c, latc, lonc, radius, radius2);

	// Compare the two radius they must be similar
	const double diff = std::fabs(radius - radius2);
	const double limit = radius / 10;

	///////TESTING////////////
	double radiusNM = radius*RAD2NM;
	double diffMt = diff * RAD2NM * 1852;
	double limitMt = limit * RAD2NM * 1852;
	radiusNM++;
	diffMt++;
	limitMt++;
	///////////////////////////////////

	return diff < limitMt;
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

Sector::Sector(const double& clat, const double& clon, const double& radiusNM, const double& dir1, const double& dir2, const bool& clockwise)
	: Geometry(LatLon(clat, clon))
	, radius(radiusNM * NM2RAD)
	, angleStart(dir1 * DEG2RAD)
	, angleEnd(dir2 * DEG2RAD)
	, couterclockwise(clockwise)
	, latc(clat * DEG2RAD)
	, lonc(-clon * DEG2RAD)
	, A(CalcRadialPoint(latc, lonc, angleStart, radius))
	, B(CalcRadialPoint(latc, lonc, angleEnd, radius)) {
}

Sector::Sector(const double& clat, const double& clon, const double& lat1, const double& lon1, const double& lat2, const double& lon2, const bool& clockwise)
	: Geometry(LatLon(clat, clon))
	, A(lat1, lon1)
	, B(lat2, lon2)
	, couterclockwise(clockwise)
	, latc(clat * DEG2RAD)
	, lonc(-clon * DEG2RAD) {
	const double lat1r = lat1 * DEG2RAD;
	const double lon1r = -lon1 * DEG2RAD;
	const double lat2r = lat2 * DEG2RAD;
	const double lon2r = -lon2 * DEG2RAD;
	radius = CalcAngularDist(latc, lonc, lat1r, lon1r);
	
	assert(radius > 0);
	assert(fabs((radius * RAD2NM) - (CalcAngularDist(latc, lonc, lat2r, lon2r) * RAD2NM)) < 0.2);
	
	angleStart = CalcGreatCircleCourse(latc, lonc, lat1r, lon1r);
	angleEnd = CalcGreatCircleCourse(latc, lonc, lat2r, lon2r);
}

bool Sector::Discretize(std::vector<LatLon>& output) const
{
	if (couterclockwise) {
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

Circle::Circle(const double& lat, const double& lon, const double& radiusNM)
	: Geometry(LatLon(lat, lon))
	, radius(radiusNM * NM2RAD)
	, latc(lat * DEG2RAD)
	, lonc(-lon * DEG2RAD) {
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
