//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2017 Alberto Realis-Luc, LK8000 team
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

#include "RasterMap.h"
#include "AirspaceConverter.h"
#include <algorithm>
#include <cassert>
#include <fstream>

RasterMap::RasterMap() :
	terrain_valid(false),
	max_field_value(0),
	DirectFine(false),
	DirectAccess(true),
	Paged(false),
	TerrainMem(nullptr) {
}

RasterMap::~RasterMap() {
	Close();
}

bool RasterMap::GetMapCenter(double *lon, double *lat) const {
	if (!terrain_valid) return false;
	*lon = (TerrainInfo.Left + TerrainInfo.Right) / 2;
	*lat = (TerrainInfo.Top + TerrainInfo.Bottom) / 2;
	return true;
}

// more accurate method
/*int RasterMap::GetEffectivePixelSize(double *pixel_D, double latitude, double longitude) const {
	double terrain_step_x, terrain_step_y;
	double step_size = TerrainInfo.StepSize*sqrt(2.0);
	if ((*pixel_D <= 0) || (step_size == 0)) {
		*pixel_D = 1.0;
		return 1;
	}
	// how many steps are in the pixel size

	//TODO DistanceBearing(latitude, longitude, latitude + step_size,longitude, &terrain_step_x, NULL);

	terrain_step_x = fabs(terrain_step_x);

	//TODO DistanceBearing(latitude, longitude, latitude,longitude + step_size, &terrain_step_y, NULL);

	terrain_step_y = fabs(terrain_step_y);
	double rfact = max(terrain_step_x, terrain_step_y) / (*pixel_D);
	int epx = (int)(max(1.0, ceil(rfact)));
	//  *pixel_D = (*pixel_D)*rfact/epx;
	return epx;
}*/


inline static int iround(double i) {
	return (int)(std::floor(i + 0.5));
}

void RasterMap::SetFieldRounding(double xr, double yr) {
	if (!terrain_valid) return;
	Xrounding = iround(xr / TerrainInfo.StepSize);
	Yrounding = iround(yr / TerrainInfo.StepSize);
	if (Xrounding<1) Xrounding = 1;
	fXrounding = 1.0 / (Xrounding*TerrainInfo.StepSize);
	fXroundingFine = fXrounding*256.0;
	if (Yrounding<1) Yrounding = 1;
	fYrounding = 1.0 / (Yrounding*TerrainInfo.StepSize);
	fYroundingFine = fYrounding*256.0;
	if ((Xrounding == 1) && (Yrounding == 1)) {
		DirectFine = true;
		xlleft = (int)(TerrainInfo.Left*fXroundingFine) + 128;
		xlltop = (int)(TerrainInfo.Top*fYroundingFine) - 128;
	} else DirectFine = false;
}

bool RasterMap::GetTerrainHeight(const double& Latitude, const double& Longitude, short& terrainHeight) const {
	if (!terrain_valid) return false;
	if (DirectFine) return GetFieldAtXY((int)(Longitude * fXroundingFine) - xlleft, xlltop - (int)(Latitude * fYroundingFine), terrainHeight);
	else {
		const unsigned int ix = ((int)((Longitude - TerrainInfo.Left) * fXrounding)) * Xrounding;
		const unsigned int iy = ((int)((TerrainInfo.Top - Latitude) * fYrounding)) * Yrounding;
		return GetFieldAtXY(ix << 8, iy << 8, terrainHeight);
	}
}

inline unsigned int CombinedDivAndMod(unsigned int &lx) {
	unsigned int ox = lx & 0xff;
	lx = lx >> 8;
	return ox;
}

bool RasterMap::GetFieldAtXY(unsigned int lx, unsigned int ly, short& result) const {
	const unsigned ix = CombinedDivAndMod(lx);
	if (lx + 1 >= TerrainInfo.Columns) return false;
	const unsigned iy = CombinedDivAndMod(ly);
	if (ly + 1 >= TerrainInfo.Rows) return false;
	const short *tm = TerrainMem + ly * TerrainInfo.Columns + lx;
	
	// perform piecewise linear interpolation
	const short &h1 = tm[0]; // (x,y)
	const short &h3 = tm[TerrainInfo.Columns + 1]; // (x+1,y+1)
	if (ix > iy) {
		// lower triangle 
		const short &h2 = tm[1]; // (x+1,y)
		result = (short)(h1 + ((ix * (h2 - h1) - iy * (h2 - h3)) >> 8));
	} else {
		// upper triangle
		const short &h4 = tm[TerrainInfo.Columns]; // (x,y+1)
		result = (short)(h1 + ((iy * (h4 - h1) - ix * (h4 - h3)) >> 8));
	}
	return true;
}

bool RasterMap::Open(const std::string& filename) {
	Close();
	std::ifstream input(filename, std::ios::in | std::ios::binary);
	if (!input.is_open() || !input.good()) {
		AirspaceConverter::LogMessage("ERROR: Falied to open raster map file: " + filename, true);
		return false;
	}
	AirspaceConverter::LogMessage("Reading raster map: " + filename, false);

	input.read((char*)&TerrainInfo, sizeof(TERRAIN_INFO));
	if (!input || input.gcount() != sizeof(TERRAIN_INFO) || TerrainInfo.StepSize == 0) {
		input.close();
		AirspaceConverter::LogMessage("ERROR: Loading raster map failed: invalid header.", true);
		return false;
	}

	unsigned int nValues = TerrainInfo.Rows * TerrainInfo.Columns;
	size_t size = nValues * sizeof(short);

	// Allocate the terrain map 
	TerrainMem = (short*)std::malloc(size);
	if (TerrainMem == nullptr) {
		input.close();
		AirspaceConverter::LogMessage("ERROR: Loading raster map failed: memory allocation failure.", true);
		assert(false);
		return false;
	}

	input.read((char*)TerrainMem, size);
	if (!input || input.gcount() != (long int)size) {
		AirspaceConverter::LogMessage("ERROR: Loading raster map failed: size doesn't match size declared in the header.", true);
		input.close();
		Close();
		return false;
	}

	for (unsigned int i = 0; i<nValues; i++) max_field_value = std::max(TerrainMem[i], max_field_value);

	terrain_valid = true;
	SetFieldRounding(0, 0);
	return true;
}

void RasterMap::Close(void) {
	terrain_valid = false;
	max_field_value = 0;
	if (TerrainMem != nullptr) {
		delete TerrainMem;
		TerrainMem = nullptr;
	}
}

bool RasterMap::PointIsInTerrainRange(const double& latitude, const double& longitude) const {
	return (latitude <= TerrainInfo.Top && latitude >= TerrainInfo.Bottom &&
		longitude >= TerrainInfo.Left && longitude <= TerrainInfo.Right);
}
