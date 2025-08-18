#include "Altitude.hpp"
#include <cmath>

const double Altitude::FEET2METER = 0.3048; // 1 Ft = 0.3048 m
const double Altitude::K1 = 0.190263;
const double Altitude::K2 = 8.417286e-5;
const double Altitude::QNE = 1013.25;
double Altitude::QNH = QNE;


Altitude::Altitude(): refIsMsl(false), fl(0), altFt(0), altMt(0), isUnlimited(false) {
}

Altitude::Altitude(const double value, bool isInMeters /* = false */, bool isMSL /* = true */):
	refIsMsl(isMSL),
	fl(0),
	altFt(isInMeters ? value / FEET2METER : value),
	altMt(isInMeters ? value : value * FEET2METER),
	isUnlimited(false) {
}

Altitude::Altitude(const int FL) :
	refIsMsl(true),
	fl(FL),
	altFt(FL * 100),
	altMt(altFt * FEET2METER),
	isUnlimited(false) {
	if (QNH != QNE) {
		altMt = QNEaltitudeToQNHaltitude(altMt);
		altFt = (int)(altMt / FEET2METER);
	}
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
	if (QNH != QNE) {
		altMt = QNEaltitudeToQNHaltitude(altMt);
		altFt = (int)(altMt / FEET2METER);
	}
}

const std::string Altitude::ToString() const {
	if (isUnlimited) return "UNLIMITED";
	if (fl > 0) return "FL" + std::to_string(fl);
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
