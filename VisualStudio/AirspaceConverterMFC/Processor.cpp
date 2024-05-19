//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2024 Alberto Realis-Luc
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

	// Set the cGPSmapper invoker function (SO specific way to invoke cGPSmapper)
	AirspaceConverter::Set_cGPSmapperFunction(std::function<bool(const std::string&, const std::string&)>(std::bind(&Processor::cGPSmapper, this, std::placeholders::_1, std::placeholders::_2)));
}

Processor::~Processor() {}

bool Processor::cGPSmapper(const std::string& polishFile, const std::string& outputFile) const {
	// Here we call cGPSmapper in the Windows way.... anyaway it works also with system(".\\cGPSmapper\\cgpsmapper.exe <args>"); on Windows as well
	AirspaceConverter::LogMessage("Invoking cGPSmapper to make: " + outputFile);

	//TODO: add arguments to create files also for other software like Garmin BaseCamp
	const std::string args(boost::str(boost::format("%1s -o %2s") % polishFile %outputFile));

	SHELLEXECUTEINFO lpShellExecInfo = { 0 };
	lpShellExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	lpShellExecInfo.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_NOCLOSEPROCESS;
	lpShellExecInfo.hwnd = NULL;
	lpShellExecInfo.lpVerb = NULL;
	lpShellExecInfo.lpFile = _com_util::ConvertStringToBSTR(converter->cGPSmapperCommand.c_str()); // _T(".\\cGPSmapper\\cgpsmapper.exe");
	lpShellExecInfo.lpParameters = _com_util::ConvertStringToBSTR(args.c_str());
	lpShellExecInfo.lpDirectory = NULL;
	lpShellExecInfo.nShow = SW_SHOW;
	lpShellExecInfo.hInstApp = (HINSTANCE)SE_ERR_DDEFAIL;
	ShellExecuteEx(&lpShellExecInfo);
	if (lpShellExecInfo.hProcess != NULL) {
		WaitForSingleObject(lpShellExecInfo.hProcess, INFINITE);
		CloseHandle(lpShellExecInfo.hProcess);
		std::remove(polishFile.c_str()); // Delete polish file
		return true;
	}
	AirspaceConverter::LogError("Failed to start cGPSmapper process.");
	return false;
}

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

bool Processor::Convert() {
	if (workerThread.joinable()) return false;
	workerThread = std::thread(std::bind(&Processor::ConvertThread, this));
	return true;
}

void Processor::ConvertThread() {
	converter->Convert();
	PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}
