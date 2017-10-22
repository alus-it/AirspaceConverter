//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2017 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <thread>
#include <map>
#include <string>
#include <vector>

#define WM_GENERAL_WORK_DONE		WM_USER+1

class AirspaceConverter;

class Processor {
public:
	Processor(HWND hwnd, AirspaceConverter* airspaceConverter);
	~Processor();
	bool LoadAirspacesFiles(const double& QNH);
	bool LoadDEMfiles();
	bool LoadWaypointsFiles();
	bool Convert();
	inline void SetWindow(HWND hwnd) { window = hwnd; }
	inline void Join() { if (workerThread.joinable()) workerThread.join(); }
	
private:
	bool cGPSmapper(const std::string& polishFile, const std::string& outputFile) const;
	void LoadAirspacesFilesThread();
	void LoadWaypointsFilesThread();
	void LoadDEMfilesThread();
	void ConvertThread();

	HWND window;
	std::thread workerThread;
	AirspaceConverter* converter;
};
