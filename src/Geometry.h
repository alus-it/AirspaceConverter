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
#include <vector>

class Airspace;
class OpenAir;

class Geometry {
friend class Airspace;
friend class OpenAir;

public:
	class LatLon {
	public:
		LatLon() : lat(UNDEF_LAT), lon(UNDEF_LON) {}
		LatLon(const double& latitude, const double& longitude) : lat(latitude), lon(longitude) {}
		inline static LatLon CreateFromRadiants(const double& latRad, const double& lonRad) { return LatLon(latRad * RAD2DEG, -lonRad * RAD2DEG); }
		inline double Lat() const { return lat; }
		inline double Lon() const { return lon; }
		inline double LatRad() const { return lat * DEG2RAD; }
		inline double LonRad() const { return -lon * DEG2RAD; }
		inline void GetLatLon(double& latitude, double& longitude) const { latitude = lat; longitude = lon; }
		inline void SetLatLon(const double& latitude, const double& longitude) { lat = latitude; lon = longitude; }
		inline void SetLat(const double& latitude) { lat = latitude; }
		inline void SetLon(const double& longitude) { lon = longitude; }
		inline void SetLatLonRad(const double latRad, const double lonRad) { lat = latRad * RAD2DEG; lon = -lonRad * RAD2DEG; }
		inline bool operator==(const LatLon& other) const { return other.lat == lat && other.lon == lon; }
		inline bool operator!=(const LatLon& other) const { return other.lat != lat || other.lon != lon; }
		inline void GetLatDegMin(int& deg, double& min) const { return convertDec2DegMin(lat, deg, min); }
		inline void GetLonDegMin(int& deg, double& min) const { return convertDec2DegMin(lon, deg, min); }
		inline void GetLatDegMinSec(int& deg, int& min, int& sec) const { return convertDec2DegMinSec(lat, deg, min, sec); }
		inline void GetLonDegMinSec(int& deg, int& min, int& sec) const { return convertDec2DegMinSec(lon, deg, min, sec); }
		inline bool GetAutoLatDegMinSec(int& deg, double& decimalMin, int& min, int& sec) const { return autoConvertDec2DegMinSec(lat, deg, decimalMin, min, sec); }
		inline bool GetAutoLonDegMinSec(int& deg, double& decimalMin, int& min, int& sec) const { return autoConvertDec2DegMinSec(lon, deg, decimalMin, min, sec); }
		inline char GetNorS() const { return lat > 0 ? 'N' : 'S'; }
		inline char GetEorW() const { return lon > 0 ? 'E' : 'W'; }
		inline bool IsValid() const { return IsValidLat(lat) &&  IsValidLon(lon); }
		inline static bool IsValidLat(const double& la) { return la >= -90 && la <= 90; }
		inline static bool IsValidLon(const double& lo) { return lo >= -180 && lo <= 180; }
		bool IsAlmostEqual(const LatLon& other) const;
		static const int UNDEF_LAT, UNDEF_LON;

	private:
		double lat, lon;
		static void convertDec2DegMin(const double& dec, int& deg, double& min);
		static void convertDec2DegMinSec(const double& dec, int& deg, int& min, int& sec);
		static bool autoConvertDec2DegMinSec(const double& dec, int& deg, double& decimalMin, int& min, int& sec);
		static const double SIXTY;
		static const double DEGTOL;
	};

	class Limits {
	public:
		Limits() : valid(false), acrossAntiGreenwich(false) {};
		Limits(const LatLon& topLeftPoint, const LatLon& bottomRightPoint) : topLeft(topLeftPoint), bottomRight(bottomRightPoint) { Verify(); }
		Limits(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon) : topLeft(topLat, leftLon), bottomRight(bottomLat, rightLon) { Verify(); }
		bool Set(const LatLon& topLeftLimit, const LatLon& bottomRightLimit);
		bool Set(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon);
		inline bool IsValid() const { return valid; }
		inline LatLon GetTopLeft() const { return topLeft; }
		inline LatLon GetBottomRight() const { return bottomRight; }
		inline double GetTopLatitudeLimit() const { return topLeft.Lat(); }
		inline double GetBottomLatitudeLimit() const { return bottomRight.Lat(); }
		inline double GetLeftLongitudeLimit() const { return topLeft.Lon(); }
		inline double GetRightLongitudeLimit() const { return bottomRight.Lon(); }
		inline void Disable() { valid = false; }
		bool IsPositionWithinLimits(const LatLon& pos) const;
		bool IsPositionWithinLimits(const double& lat, const double& lon) const;

	private:
		void Verify();
		LatLon topLeft;
		LatLon bottomRight;
		bool valid;
		bool acrossAntiGreenwich;
	};

	virtual ~Geometry() {}
	virtual bool Discretize(std::vector<LatLon>& output) const = 0;
	static inline void SetResolution(const double resolutionNM) { resolution = resolutionNM * NM2RAD; }
	static bool CalcAirfieldPolygon(const double lat, const double lon, const int length, const int dir, std::vector<LatLon>& polygon);
	inline const LatLon& GetCenterPoint() const { return point; }

	static const double NM2M, MI2M;

protected:
	Geometry(const LatLon& center) : point(center) {}
	const LatLon point;
	static double resolution; // [rad] maximun distance between points when discretizing
	static const double TWO_PI;
	static const double PI_2;
	static const double DEG2RAD;
	static const double RAD2DEG;
	static const double NM2RAD;
	static const double RAD2NM;
	static const double M2RAD;

	static double FindStep(const double& radius, const double& angle);
	static double DeltaAngle(const double angle, const double reference);
	static double AbsAngle(const double& angle);
	static double AnglePi2Pi(const double& angle);
	static double CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& d);
	static double CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2);
	static double CalcAngularDist(const double& lat1, const double& lon1, const double& lat2, const double& lon2);
	static void CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst, double& lat, double& lon);
	static LatLon CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst);
	static bool CalcBisector(const double& latA, const double& lonA, const double& latB, const double& lonB, const double& latC, const double& lonC, double& bisector);
	static void CalcSphericalTriangle(const double& a, const double& beta, const double& gamma, double& alpha, double& b, double& c);
	static bool CalcRadialIntersection(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& crs13, const double& crs23, double& lat3, double& lon3, double& dst13, double& dst23);
	static bool ArePointsOnArc(const LatLon& A, const LatLon& B, const LatLon& C, double& latc, double& lonc, double& radius, bool& clockwise);
	static LatLon AveragePoints(const std::vector<std::pair<const double, const double>>& centerPoints);
	static double AverageRadius(const Geometry::LatLon& center, const std::vector<LatLon*>& circlePoints);
	static double RoundDistanceInNM(const double radiusRad);
	static bool IsInt(const double& number, int& intVal);

private:
	static const double PI;
	static const double TOL;
	virtual void WriteOpenAirGeometry(OpenAir& openAir) const = 0;
	virtual bool IsPoint() const = 0;
};

class Point : public Geometry {
friend class OpenAir;

public:
	Point(const LatLon& latlon) : Geometry(latlon) {}
	Point(const double& lat, const double& lon) : Geometry(LatLon(lat,lon)) {}
	bool Discretize(std::vector<LatLon>& output) const;

private:
	void WriteOpenAirGeometry(OpenAir& openAir) const;
	inline bool IsPoint() const { return true; }
};

class Sector : public Geometry {
friend class OpenAir;

public:
	Sector(const LatLon& center, const double radiusNM, const double dir1, const double dir2, const bool isClockwise);
	Sector(const LatLon& center, const LatLon& pointStart, const LatLon& pointEnd, const bool isClockwise);
	bool Discretize(std::vector<LatLon>& output) const;
	inline double GetRadiusNM() const { return RAD2NM * radius; }
	inline bool IsClockwise() const { return clockwise; }
	inline const LatLon& GetStartPoint() const { return A; }
	inline const LatLon& GetEndPoint() const { return B; }
	inline double GetAngleStart() const { return RAD2DEG * angleStart; }
	inline double GetAngleEnd() const { return RAD2DEG * angleEnd; }

private:
	void WriteOpenAirGeometry(OpenAir& openAir) const;
	inline bool IsPoint() const { return false; }

	const bool clockwise;
	const double latc, lonc; // [rad]
	double angleStart, angleEnd; // [rad]
	double radius; // [rad]
	LatLon A, B;
};

class Circle : public Geometry {
friend class OpenAir;

public:
	Circle(const LatLon& center, const double& radiusNM);
	bool Discretize(std::vector<LatLon>& output) const;
	inline double GetRadiusNM() const { return RAD2NM * radius; }

private:
	void WriteOpenAirGeometry(OpenAir& openAir) const;
	inline bool IsPoint() const { return false; }

	const double radius; // [rad]
	const double latc, lonc; // [rad]
};

/* Airway for now not supported
class AirwayPoint : public Geometry
{
public:
	AirwayPoint(const double& lat, const double& lon, const double& widthNM) : Geometry(LatLon(lat, lon)), width(widthNM) {}
	bool Discretize(std::vector<LatLon>& output) const;

private:
	double width;
};
*/
