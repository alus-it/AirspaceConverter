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

class Altitude
{
public:
	//Altitude() { refIsMsl = true; altMt = 0; altFt = 0; fl = 0; }
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

private:
	static double QNEaltitudeToStaticPressure(const double alt);
	static double StaticPressureToQNHaltitude(const double ps);
	static double QNEaltitudeToQNHaltitude(const double ps);

	bool refIsMsl;
	double altMt;
	int altFt, fl;
	static const double FEET2METER, K1, K2, QNE;
	static double QNH;
};

class LatLon
{
public:
	inline LatLon() : lat(UNDEF_LAT), lon(UNDEF_LON) {}
	inline LatLon(const double& latitude, const double& longitude) : lat(latitude), lon(longitude) {}
	inline double Lat() const { return lat; }
	inline double Lon() const { return lon; }
	inline void GetLatLon(double& latitude, double& longitude) const { latitude = lat; longitude = lon; }
	inline void SetLatLon(double& latitude, double& longitude) { lat = latitude; lon = longitude; }
	inline bool operator==(const LatLon& other) const { return other.lat == lat && other.lon == lon; }
	inline bool operator!=(const LatLon& other) const { return other.lat != lat || other.lon != lon; }
	inline void GetLatDegMin(int& deg, double& min) const { return convertDec2DegMin(lat, deg, min); }
	inline void GetLonDegMin(int& deg, double& min) const { return convertDec2DegMin(lon, deg, min); }
	inline char GetNorS() const { return lat > 0 ? 'N' : 'S'; }
	inline char GetEorW() const { return lon > 0 ? 'E' : 'W'; }

	static const int UNDEF_LAT, UNDEF_LON;

private:
	double lat, lon;
	static void convertDec2DegMin(const double& dec, int& deg, double& min);
	static const double SIXTYTH;
};

class Geometry
{
public:
	virtual ~Geometry() {}
	virtual bool Discretize(std::vector<LatLon>& output) const = 0;
	static inline void SetResolution(double resolutionNM) { resolution = resolutionNM * NM2RAD; }

protected:
	inline Geometry(const LatLon& center) : point(center) {}
	const LatLon point;
	static double resolution; // [rad] maximun distance between points when discretizing 

	static const double TWO_PI;
	static const double DEG2RAD;
	static const double RAD2DEG;
	static const double NM2RAD;
	static const double RAD2NM;
	
	static double FindStep(const double& radius, const double& angle);
	static double AbsAngle(const double& angle);
	static double CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& d);
	static double CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2);
	static double CalcAngularDist(const double& lat1, const double& lon1, const double& lat2, const double& lon2);
	static LatLon CalculateRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst);

private:
	static const double PI;
	static const double PI_2;
};

class Point : public Geometry
{
public:
	inline Point(const LatLon& latlon) : Geometry(latlon) {}
	inline Point(const double& lat, const double& lon) : Geometry(LatLon(lat,lon)) {}
	inline ~Point() {}
	bool Discretize(std::vector<LatLon>& output) const;
};

class Sector : public Geometry
{
public:
	Sector(const double& clat, const double& clon, const double& radiusNM, const double& dir1, const double& dir2, const bool& clocwise);
	Sector(const double& clat, const double& clon, const double& lat1, const double& lon1, const double& lat2, const double& lon2, const bool& clocwise);
	inline ~Sector() {}
	bool Discretize(std::vector<LatLon>& output) const;

private:
	double radius; // [rad]
	double angleStart, angleEnd; // [rad]
	const bool couterclockwise;
	const double latc, lonc; // [rad]
};

class Circle : public Geometry
{
public:
	Circle(const double& lat, const double& lon, const double& radiusNM);
	inline ~Circle() {}
	bool Discretize(std::vector<LatLon>& output) const;

private:
	const double radius; // [rad]
	const double latc, lonc; // [rad]
};

/* Airway for now not supported
class AirwayPoint : public Geometry
{
public:
	inline AirwayPoint(const double& lat, const double& lon, const double& widthNM) : Geometry(LatLon(lat, lon)), width(widthNM) {}
	inline ~AirwayPoint() {}
	bool Discretize(std::vector<LatLon>& output) const;

private:
	double width;
};

*/

class Airspace
{
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
		UNKNOWN //also the last one
	} Type;

	inline Airspace() : type(UNKNOWN) {}
	inline Airspace(Type category) : type(category) {}
	Airspace(Airspace&& orig);
	~Airspace();

	inline static const std::string& CategoryName(const Type& category) { return CATEGORY_NAMES[category]; }
	static const bool CategoryVisibleByDefault(const Type& category) { return CATEGORY_VISIBILITY[category]; }

	inline void SetType(const Type& category) { type = category; }
	inline void SetTopAltitude(const Altitude& alt) { top = alt; }
	inline void SetBaseAltitude(const Altitude& alt) { base = alt; }
	inline void SetName(const std::string& airspaceName) { name = airspaceName; }
	inline void AddSinglePointOnly(const double& lat, const double& lon) { points.push_back(LatLon(lat, lon)); }
	void Clear();
	void AddPoint(const double& lat, const double& lon);
	void AddGeometry(const Geometry* geometry);

	inline void ClosePoints() { if (points.front() != points.back()) points.push_back(points.front()); }
	void Discretize();

	inline const Type& GetType() const { return type; }
	inline const std::string& GetCategoryName() const { return CategoryName(type); }
	inline const Altitude& GetTopAltitude() const { return top; }
	inline const Altitude& GetBaseAltitude() const { return base; }
	inline const std::string& GetName() const { return name; }
	inline const unsigned int GetNumberOfGeometries() const { return geometries.size(); }
	inline const std::vector<LatLon>& GetPoints() const { return points; }
	inline const LatLon& GetFirstPoint() const { return points.front(); }
	inline const LatLon& GetLastPoint() const { return points.back(); }
	inline const unsigned int GetNumberOfPoints() const { return points.size(); }
	inline const LatLon& GetPointAt(unsigned int pos) const { return points.at(pos); }
	inline const bool IsGNDbased() const { return base.IsGND(); }
	inline const bool IsVisibleByDefault() const { return CategoryVisibleByDefault(type); }

private:
	static const std::string CATEGORY_NAMES[];
	static const bool CATEGORY_VISIBILITY[];
	Altitude top, base;
	std::vector<const Geometry*> geometries;
	std::vector<LatLon> points;
	Type type;
	std::string name;
};
