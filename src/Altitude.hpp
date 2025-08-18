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

#pragma once
#include <string>

class Altitude {
public:
	Altitude();
	Altitude(const double value, bool isInMeters = false, bool isMSL = true);
    Altitude(const int FL);

	inline void SetAltFt(const int ft, const bool isAMSL = true) { refIsMsl = isAMSL; altMt = ft*FEET2METER; altFt = ft; fl = 0; isUnlimited = false; }
	inline void SetAltMt(const double mt, const bool isAMSL = true) { refIsMsl = isAMSL;  altFt = (int)(mt / FEET2METER); altMt = mt; fl = 0; isUnlimited = false; }
	void SetFlightLevel(const int FL);
	inline void SetGND() { refIsMsl = false; altMt = 0; altFt = 0; fl = 0; isUnlimited = false; }
	inline void SetUnlimited() { SetFlightLevel(600); isUnlimited = true; }
	inline bool IsAMSL() const { return refIsMsl; }
	inline bool IsAGL() const { return !refIsMsl; }
	inline bool IsFL() const { return fl != 0; }
	inline bool IsUnlimited() const { return isUnlimited; }
	inline int GetAltFt() const { return altFt; }
	inline double GetAltMt() const { return altMt; }
	inline bool IsGND() const { return !refIsMsl && altFt == 0; }
	inline bool IsMSL() const { return refIsMsl && altFt == 0; }
	bool operator<(const Altitude& other) const;
	bool operator>(const Altitude& other) const;
	bool operator>=(const Altitude& other) const;
	bool operator<=(const Altitude& other) const;
	bool operator==(const Altitude& other) const;
	bool operator!=(const Altitude& other) const;
	
	const std::string ToString() const;
	inline static void SetQNH(const double QNHmb) { QNH = QNHmb; }
	inline static double GetQNH() { return QNH; }
	static const double FEET2METER;

private:
	static double QNEaltitudeToStaticPressure(const double alt);
	static double StaticPressureToQNHaltitude(const double ps);
	static double QNEaltitudeToQNHaltitude(const double ps);

	bool refIsMsl;
	int fl; // Flight level
	int altFt; // Alt in feet
	double altMt; // Alt in meters
	bool isUnlimited;
	static const double K1, K2, QNE;
	static double QNH;
};
