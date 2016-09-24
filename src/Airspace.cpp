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
#include <cmath>
#include <algorithm>
#include <cassert>

const double Altitude::FEET2METER = 0.3048; // 1 Ft = 0.3048 m
const double Altitude::K1 = 0.190263;
const double Altitude::K2 = 8.417286e-5;
const double Altitude::QNE = 1013.25;
double Altitude::QNH = QNE;

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

Airspace::Airspace(Airspace&& orig) // Move constructor
	: top(std::move(orig.top))
	, base(std::move(orig.base))
	, geometries(std::move(orig.geometries))
	, points(std::move(orig.points))
	, type(std::move(orig.type))
	, name(std::move(orig.name)) {
	orig.type = UNDEFINED;
}

Airspace::~Airspace() {
	for (const Geometry* g : geometries) delete g;
	geometries.clear();
}

void Airspace::Clear() {
	type = UNDEFINED;
	name.clear();
	for (const Geometry* g : geometries) delete g;
	geometries.clear();
	points.clear();
}

void Airspace::AddPoint(const Geometry::LatLon& point) {
	geometries.push_back(new Point(point));
	points.push_back(point);
}

void Airspace::AddGeometry(const Geometry* geometry) {
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
		const Geometry::LatLon center(Geometry::AveragePoints(centerPoints));
		geometries.push_back(new Circle(center, Geometry::AverageRadius(center, arcPoints) * Geometry::RAD2NM));
	} else for (const Geometry::LatLon* p : arcPoints) geometries.push_back(new Point(*p));
}

bool Airspace::Undiscretize() {
	if (!geometries.empty()) return true;
	if (points.empty()) return false;
	assert(points.size() >= 4);
	unsigned int steps = points.size() - 3;
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
	} else for (unsigned int i = steps - 1; i < points.size() ; i++) geometries.push_back(new Point(points.at(i))); // Otherwise the remaining points
	return true;
}
