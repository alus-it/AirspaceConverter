//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
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
	inline void SetAltFtMSL(const int ft) { refIsMsl = true; altMt = ft*FEET2METER; altFt = ft; fl = 0; }
	inline void SetAltMtMSL(const double mt) { refIsMsl = true;  altFt = (int)(mt / FEET2METER); altMt = mt; fl = 0; }
	inline void SetAltFtGND(const int ft) { refIsMsl = false; altMt = ft*FEET2METER; altFt = ft; fl = 0; }
	inline void SetAltMtGND(const double mt) { refIsMsl = false; altFt = (int)(mt / FEET2METER); altMt = mt; fl = 0; }
	void SetFlightLevel(const int FL);
	inline void SetGND() { refIsMsl = false; altMt = 0; altFt = 0; fl = 0; }
	inline bool IsAMSL() const { return refIsMsl; }
	inline bool IsAGL() const { return !refIsMsl; }
	inline bool IsFL() const { return fl != 0; }
	inline int GetAltFt() const { return altFt; }
	inline double GetAltMt() const { return altMt; }
	inline bool IsGND() const { return !refIsMsl && altFt == 0; }
	const std::string ToString() const;
	inline static void SetQNH(const double QNHmb) { QNH = QNHmb; }
	inline static double GetQNH() { return QNH; }
	static const double FEET2METER;

private:
	static double QNEaltitudeToStaticPressure(const double alt);
	static double StaticPressureToQNHaltitude(const double ps);
	static double QNEaltitudeToQNHaltitude(const double ps);

	bool refIsMsl;
	double altMt;
	int altFt, fl;
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

	inline Airspace() : type(UNDEFINED) {}
	inline Airspace(Type category) : type(category) {}
	Airspace(Airspace&& orig);
	~Airspace();

	inline static const std::string& CategoryName(const Type& category) { return CATEGORY_NAMES[category]; }
	static const bool CategoryVisibleByDefault(const Type& category) { return CATEGORY_VISIBILITY[category]; }
	inline void SetType(const Type& category) { type = category; }
	inline void SetTopAltitude(const Altitude& alt) { top = alt; }
	inline void SetBaseAltitude(const Altitude& alt) { base = alt; }
	inline void SetName(const std::string& airspaceName) { name = airspaceName; }
	inline void AddSinglePointOnly(const double& lat, const double& lon) { points.push_back(Geometry::LatLon(lat, lon)); }
	void Clear();
	void AddPoint(const Geometry::LatLon& point);
	void AddGeometry(const Geometry* geometry);
	inline void ClosePoints() { if (points.front() != points.back()) points.push_back(points.front()); }
	bool Undiscretize();
	inline const Type& GetType() const { return type; }
	inline const std::string& GetCategoryName() const { return CategoryName(type); }
	inline const Altitude& GetTopAltitude() const { return top; }
	inline const Altitude& GetBaseAltitude() const { return base; }
	inline const std::string& GetName() const { return name; }
	inline const unsigned int GetNumberOfGeometries() const { return geometries.size(); }
	inline const Geometry* GetGeometryAt(unsigned int i) { return i < geometries.size() ? geometries.at(i) : nullptr; }
	inline const std::vector<Geometry::LatLon>& GetPoints() const { return points; }
	inline const Geometry::LatLon& GetFirstPoint() const { return points.front(); }
	inline const Geometry::LatLon& GetLastPoint() const { return points.back(); }
	inline const unsigned int GetNumberOfPoints() const { return points.size(); }
	inline const Geometry::LatLon& GetPointAt(unsigned int pos) const { return points.at(pos); }
	inline const bool IsGNDbased() const { return base.IsGND(); }
	inline const bool IsVisibleByDefault() const { return CategoryVisibleByDefault(type); }

private:
	void EvaluateAndAddArc(std::vector<Geometry::LatLon*>& arcPoints, std::vector<std::pair<const double, const double>>& centerPoints, const bool& clockwise);
	void EvaluateAndAddCircle(const std::vector<Geometry::LatLon*>& arcPoints, const std::vector<std::pair<const double, const double>>& centerPoints);

	static const std::string CATEGORY_NAMES[];
	static const bool CATEGORY_VISIBILITY[];
	Altitude top, base;
	std::vector<const Geometry*> geometries;
	std::vector<Geometry::LatLon> points;
	Type type;
	std::string name;
};
