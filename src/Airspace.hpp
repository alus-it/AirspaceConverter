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
#include <vector>
#include "Geometry.hpp"
#include "Altitude.hpp"

class Airspace {
public:
	typedef enum {
		CLASSA = 0,	// Airspace class A
		CLASSB,		// Airspace class B
		CLASSC,		// Airspace class C
		CLASSD,		// Airspace class D
		CLASSE,		// Airspace class E
		CLASSF,		// Airspace class F
		CLASSG,		// Airspace class G
		D,			// Danger area
		P,			// Prohibited area
		R,			// Restricted area
		CTR,		// Control Traffic Region
		TMZ,		// Transponder Mandatory Zone
		RMZ,		// Radio Mandatory Zone
		GLIDING,	// Gliding area
		NOGLIDER,	// No gliding area
		WAVE,		// Wave window
		NOTAM,		// Notice to Airmen "NOTAM" airspace category can be used in OpenAir files
		OTHER,		// Generic other type
		TMA,		// Terminal Manoeuvering Area
		FIR,		// Flight Information Region, from here on not visible by default
		UIR,		// Upper Information Region
		OTH,		// Over The Horizon
		AWY,		// Airway
		MATZ,		// Military Aerodrome Traffic Zone
		MTMA,		// Military Terminal Zone
		MTRA,		// Military Temporary Reserved Area
		TFR,		// Temporary Flight Restriction
		ADA,		// Advisory Area
		ADIZ,		// Air Defense Identification Zone
		CTA,		// Control Area
		DFIR,		// Delegated FIR
		TIZ,		// Traffic Information Zone
		TIA,		// Traffic Information Area
		SRZ,		// Special Rules Zone
		ATZ,		// Aerodrome Traffic Zone
		FISA,		// Flight Information Service Area
		MBZ,		// Mandatory Broadcast Zone
		ASR,		// Aerial Sporting and Recreation Area
		COMP,		// Competition boundary
		TRZ,		// Transponder Recommended Zone
		VFRR,		// VFR Route
		RTZ,		// Radio/Transponder Mandatory zone
		PARA,		// Parachute jumping area
		LFZ,		// Low Flying Zone
		CFZ,		// Common Frequency Zone
		MOA,		// Military Operating Area
		MTA,		// Military Training Area
		TSA,		// Temporary segregated airspace
		TRA,		// Temporary reserved airspace
		UNKNOWN,	// "UNKNOWN" as well can be used in OpenAir files
		UNDEFINED	// also the last one
	} Type;

	Airspace() : type(UNDEFINED), airspaceClass(UNDEFINED), transponderCode(-1) {}
	Airspace(Type category);
	Airspace(const Airspace& orig);
	Airspace(Airspace&& orig);
	~Airspace();

	Airspace& operator=(const Airspace& other);
	bool operator==(const Airspace& other) const;
	inline static const std::string& CategoryName(const Type& category) { return CATEGORY_NAMES[category]; }
	inline static const std::string& LongCategoryName(const Type& category) { return LONG_CATEGORY_NAMES[category]; }
	static bool CategoryVisibleByDefault(const Type& category) { return CATEGORY_VISIBILITY[category]; }
	void SetType(const Type& category);
	void SetClass(const Type& airspClass);
	bool GuessClassFromName();
	bool NameStartsWithIdent(const std::string& ident);
	inline void SetTopAltitude(const Altitude& alt) { top = alt; }
	inline void SetBaseAltitude(const Altitude& alt) { base = alt; }
	inline void SetName(const std::string& airspaceName) { name = airspaceName; }
	bool SetTransponderCode(const std::string& code);
	void AddRadioFrequency(const int frequencyHz, const std::string& description);
	void Clear(); // Clear name, type, points and geometries
	void ClearPoints(); // Clear points and geometries
	void ClearGeometries(); // Clear geometries only
	bool AddPoint(const Geometry::LatLon& point);
	bool AddPointLatLonOnly(const double& lat, const double& lon);
	void AddGeometry(const Geometry* geometry);
	bool ClosePoints();
	bool ArePointsValid() const;
	void RemoveTooCloseConsecutivePoints();
	bool Undiscretize();
	bool IsWithinLatLonLimits(const Geometry::Limits& limits) const;
	bool IsWithinAltLimits(const Altitude& floor, const Altitude& ceil) const;
	inline void CutPointsFrom(Airspace& orig) { points = std::move(orig.points); }
	inline const Type& GetType() const { return type; }
	inline const Type& GetClass() const { return airspaceClass; }
	inline const std::string& GetCategoryName() const { return CategoryName(type); }
	inline const std::string& GetLongCategoryName() const { return LongCategoryName(type); }
	inline const Altitude& GetTopAltitude() const { return top; }
	inline const Altitude& GetBaseAltitude() const { return base; }
	inline const std::string& GetName() const { return name; }
	inline size_t GetNumberOfGeometries() const { return geometries.size(); }
	inline const Geometry* GetGeometryAt(size_t i) { return i < geometries.size() ? geometries.at(i) : nullptr; }
	inline const std::vector<Geometry::LatLon>& GetPoints() const { return points; }
	inline const Geometry::LatLon& GetFirstPoint() const { return points.front(); }
	inline const Geometry::LatLon& GetLastPoint() const { return points.back(); }
	inline size_t GetNumberOfPoints() const { return points.size(); }
	inline const Geometry::LatLon& GetPointAt(size_t pos) const { return points.at(pos); }
	inline bool IsGNDbased() const { return base.IsGND(); }
	inline bool IsMSLbased() const { return base.IsMSL(); }
	inline bool IsAGLtopped() const { return top.IsAGL(); }
	inline bool IsAMSLtopped() const { return top.IsAMSL(); }
	inline bool IsVisibleByDefault() const { return CategoryVisibleByDefault(type); }
	inline size_t GetNumberOfRadioFrequencies() const { return radioFrequencies.size(); }
	inline const std::pair<int, std::string>& GetRadioFrequencyAt(size_t pos) const { return radioFrequencies.at(pos); }
	std::string GetTransponderCode() const;
	inline bool HasTransponderCode() const { return transponderCode >= 0; }
	void CalculateSurface(double& areaKm2, double& perimeterKm) const;

private:
	bool AddPointGeometryOnly(const Geometry::LatLon& point);
	void EvaluateAndAddArc(std::vector<Geometry::LatLon*>& arcPoints, std::vector<std::pair<const double, const double>>& centerPoints, const bool& clockwise);
	void EvaluateAndAddCircle(const std::vector<Geometry::LatLon*>& arcPoints, const std::vector<std::pair<const double, const double>>& centerPoints);

	static const std::string CATEGORY_NAMES[];
	static const std::string LONG_CATEGORY_NAMES[];
	static const bool CATEGORY_VISIBILITY[];
	Altitude top, base;
	std::vector<const Geometry*> geometries;
	std::vector<Geometry::LatLon> points;
	Type type;
	Type airspaceClass; // This is to remember the class of a TMA or CTR where possible
	std::string name;
	std::vector<std::pair<int,std::string>> radioFrequencies; // Radio frequencies list values expressed in [Hz] and name/description
	short transponderCode; // Transponder code mandated for this airspace 12 bits used (OCT:7777 = DEC:4095 = BIN:1111111111)
};
