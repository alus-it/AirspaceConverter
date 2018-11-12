//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2018 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <string>
#include <vector>
#include "Geometry.h"

class Altitude {
public:
	Altitude() : refIsMsl(false), fl(0), altFt(0), altMt(0), isUnlimited(false) {}
	//Altitude(const int feet, const bool isAMSL) : refIsMsl(isAMSL), fl(0), altFt(feet), altMt(feet * FEET2METER), isUnlimited(false) {}
	//Altitude(const double meters, const bool isAMSL) : refIsMsl(isAMSL), fl(0), altFt((int)(meters / FEET2METER)), altMt(meters), isUnlimited(false) {}
	//Altitude(const int FL);

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

class Airspace {
public:
	typedef enum {
		CLASSA = 0,
		CLASSB,
		CLASSC,
		CLASSD,
		CLASSE,
		CLASSF,
		CLASSG,
		DANGER,
		PROHIBITED,
		RESTRICTED,
		CTR,
		TMA,
		TMZ,
		RMZ,
		FIR, // from here on not visible by default
		UIR,
		OTH,
		GLIDING,
		NOGLIDER,
		WAVE,
		UNKNOWN, // "UNKNOWN" can be used in OpenAir files
		UNDEFINED // also the last one
	} Type;

	Airspace() : type(UNDEFINED) , airspaceClass(UNDEFINED) {}
	Airspace(Type category);
	Airspace(const Airspace& orig);
	Airspace(Airspace&& orig);
	~Airspace();

	Airspace& operator=(const Airspace& other);
	bool operator==(const Airspace& other) const;
	inline static const std::string& CategoryName(const Type& category) { return CATEGORY_NAMES[category]; }
	static bool CategoryVisibleByDefault(const Type& category) { return CATEGORY_VISIBILITY[category]; }
	void SetType(const Type& category);
	void SetClass(const Type& airspClass);
	bool GuessClassFromName();
	bool NameStartsWithIdent(const std::string& ident);
	inline void SetTopAltitude(const Altitude& alt) { top = alt; }
	inline void SetBaseAltitude(const Altitude& alt) { base = alt; }
	inline void SetName(const std::string& airspaceName) { name = airspaceName; }
	inline void AddSinglePointOnly(const double& lat, const double& lon) { points.push_back(Geometry::LatLon(lat, lon)); }
	void Clear(); // Clear name, type, points and geometries
	void ClearPoints(); // Clear points and geometries
	void ClearGeometries(); // Clear geometries only
	void AddPoint(const Geometry::LatLon& point);
	void AddGeometry(const Geometry* geometry);
	inline void ClosePoints() { if (!points.empty() && points.front() != points.back()) points.push_back(points.front()); }
	bool ArePointsValid() const;
	bool Undiscretize();
	bool IsWithinLimits(const Geometry::Limits& limits) const;
	inline void CutPointsFrom(Airspace& orig) { points = std::move(orig.points); }
	inline const Type& GetType() const { return type; }
	inline const Type& GetClass() const { return airspaceClass; }
	inline const std::string& GetCategoryName() const { return CategoryName(type); }
	inline const Altitude& GetTopAltitude() const { return top; }
	inline const Altitude& GetBaseAltitude() const { return base; }
	inline const std::string& GetName() const { return name; }
	inline unsigned int GetNumberOfGeometries() const { return (unsigned int)geometries.size(); }
	inline const Geometry* GetGeometryAt(unsigned int i) { return i < geometries.size() ? geometries.at(i) : nullptr; }
	inline const std::vector<Geometry::LatLon>& GetPoints() const { return points; }
	inline const Geometry::LatLon& GetFirstPoint() const { return points.front(); }
	inline const Geometry::LatLon& GetLastPoint() const { return points.back(); }
	inline unsigned int GetNumberOfPoints() const { return (unsigned int)points.size(); }
	inline const Geometry::LatLon& GetPointAt(unsigned int pos) const { return points.at(pos); }
	inline bool IsGNDbased() const { return base.IsGND(); }
	inline bool IsMSLbased() const { return base.IsMSL(); }
	inline bool IsAGLtopped() const { return top.IsAGL(); }
	inline bool IsAMSLtopped() const { return top.IsAMSL(); }
	inline bool IsVisibleByDefault() const { return CategoryVisibleByDefault(type); }
	void CalculateSurface(double& areaKm2, double& perimeterKm) const;

private:
	void EvaluateAndAddArc(std::vector<Geometry::LatLon*>& arcPoints, std::vector<std::pair<const double, const double>>& centerPoints, const bool& clockwise);
	void EvaluateAndAddCircle(const std::vector<Geometry::LatLon*>& arcPoints, const std::vector<std::pair<const double, const double>>& centerPoints);

	static const std::string CATEGORY_NAMES[];
	static const bool CATEGORY_VISIBILITY[];
	Altitude top, base;
	std::vector<const Geometry*> geometries;
	std::vector<Geometry::LatLon> points;
	Type type;
	Type airspaceClass; // This is to remember the class of a TMA or CTR where possible
	std::string name;
};
