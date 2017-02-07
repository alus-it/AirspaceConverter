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

#include "stdafx.h"
#include "Processor.h"
#include "AirspaceConverter.h"
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

Processor::Processor(HWND hwnd, AirspaceConverter* airspaceConverter) :
	window(hwnd) ,
	converter(airspaceConverter) {
	assert(converter != nullptr);
}

Processor::~Processor() {}

bool Processor::LoadAirspacesFiles(const double& newQNH) {
	if (workerThread.joinable()) return false;
	converter->SetQNH(newQNH);
	workerThread = std::thread(std::bind(&Processor::LoadAirspacesFilesThread, this));
	return true;
}

void Processor::LoadAirspacesFilesThread() {
	converter->LoadAirspaces();
	PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}

bool Processor::LoadDEMfiles() {
	if (workerThread.joinable()) return false;
	workerThread = std::thread(std::bind(&Processor::LoadDEMfilesThread, this));
	return true;
}

void Processor::LoadDEMfilesThread() {
	converter->LoadTerrainRasterMaps();
	PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}

bool Processor::LoadWaypointsFiles() {
	if (workerThread.joinable()) return false;
	workerThread = std::thread(std::bind(&Processor::LoadWaypointsFilesThread, this));
	return true;
}

void Processor::LoadWaypointsFilesThread() {
	converter->LoadWaypoints();
	PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}

bool Processor::Convert(const std::string& outputFilename, const int type)
{
	if (workerThread.joinable()) return false;
	workerThread = std::thread(std::bind(&Processor::ConvertThread, this, outputFilename, type));
	return true;
}

void Processor::ConvertThread(const std::string& outputFile, const int type)
{
	if (converter->Convert(outputFile, (AirspaceConverter::OutputType)type) && type == AirspaceConverter::Garmin) {
	
		// Special case: Garmin IMG need to call cGPSmapper
		// For now we have done only the Polish file in the lib...
		const std::string polishFile(boost::filesystem::path(outputFile).replace_extension(".mp").string());
		
		// Here we call cGPSmapper in the Windows way....
		// TODO: all that SO specific stuff the should be a function to be used directly in the lib with functional
		AirspaceConverter::LogMessage("Invoking cGPSmapper to make: " + outputFile, false);
		
		//TODO: add arguments to create files also for other software like Garmin BaseCamp
		const std::string args(boost::str(boost::format("%1s -o %2s") %polishFile %outputFile));
		
		SHELLEXECUTEINFO lpShellExecInfo = { 0 };
		lpShellExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		lpShellExecInfo.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_NOCLOSEPROCESS;
		lpShellExecInfo.hwnd = NULL;
		lpShellExecInfo.lpVerb = NULL;
		lpShellExecInfo.lpFile = _T(".\\cGPSmapper\\cgpsmapper.exe");
		lpShellExecInfo.lpParameters = _com_util::ConvertStringToBSTR(args.c_str());
		lpShellExecInfo.lpDirectory = NULL;
		lpShellExecInfo.nShow = SW_SHOW;
		lpShellExecInfo.hInstApp = (HINSTANCE)SE_ERR_DDEFAIL;
		ShellExecuteEx(&lpShellExecInfo);
		if (lpShellExecInfo.hProcess == NULL) AirspaceConverter::LogMessage("ERROR: Failed to start cGPSmapper process.", true);
		else {
			WaitForSingleObject(lpShellExecInfo.hProcess, INFINITE);
			CloseHandle(lpShellExecInfo.hProcess);
			std::remove(polishFile.c_str()); // Delete polish file
		}
	}
	PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}
