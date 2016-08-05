//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc, LK8000 team
// License     : GNU GPL v3
//
// The source code in this file was adopted from LK8000 project
//
//   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
//   Released under GNU/GPL License v.2
//   See CREDITS.TXT file in LK8000 project for authors and copyrights
//   LK8000 Repository: https://github.com/LK8000/LK8000.git
//
//============================================================================

#pragma once
#include <string>

class RasterMap final {
public:
	typedef struct _TERRAIN_INFO
	{
		double Left;
		double Right;
		double Top;
		double Bottom;
		double StepSize;
		uint32_t Rows;
		uint32_t Columns;
	} TERRAIN_INFO;

	RasterMap();
	~RasterMap();
	inline bool isMapLoaded() const { return terrain_valid; }
	bool GetMapCenter(double *lon, double *lat) const;
	inline double GetStepSize() const { return TerrainInfo.StepSize; }
	inline double GetTop() const { return TerrainInfo.Top; }
	inline double GetBottom() const { return TerrainInfo.Bottom; }
	//int GetEffectivePixelSize(double *pixel_D, double latitude, double longitude) const; // accurate method
	void SetFieldRounding(double xr, double yr);
	bool GetTerrainHeight(const double& Latitude, const double& Longitude, short& terrainHeight) const;
	bool Open(const std::string& filename);
	void Close();
	//void Lock();
	//void Unlock();
	inline bool IsDirectAccess(void) const { return DirectAccess; };
	inline bool IsPaged(void) const { return Paged; };
	bool PointIsInTerrainRange(const double& latitude, const double& longitude) const;

private:
	bool GetFieldAtXY(unsigned int lx, unsigned int ly, short& result) const;
	bool terrain_valid;
	short max_field_value;
	TERRAIN_INFO TerrainInfo;
	int xlleft;
	int xlltop;
	bool DirectFine;
	bool DirectAccess;
	bool Paged;
	double fXrounding, fYrounding;
	double fXroundingFine, fYroundingFine;
	int Xrounding, Yrounding;
	short* TerrainMem;
};
