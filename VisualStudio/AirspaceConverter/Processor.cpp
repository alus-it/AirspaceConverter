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
#include "OpenAIPreader.h"
#include "Airspace.h"
#include "KMLwriter.h"
#include "PFMwriter.h"
#include "OpenAir.h"
#include "CUPreader.h"
#include "Waypoint.h"
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

Processor::Processor(HWND hwnd) :
	window(hwnd),
	//abort(false),
	outputType(AirspaceConverter::NumOfOutputTypes) {
}

Processor::~Processor() {
	//Abort();
	KMLwriter::ClearTerrainMaps();
	UnloadWaypoints();
}

bool Processor::AddInputFile(const std::string& inputFile)
{
	std::string ext(boost::filesystem::path(inputFile).extension().string());
	if (boost::iequals(ext, ".aip")) {
		openAIPinputFiles.push_back(inputFile);
		return true;
	}
	if (boost::iequals(ext, ".txt")) {
		openAirInputFiles.push_back(inputFile);
		return true;
	}
	return false;
}

bool Processor::LoadAirspacesFiles(const double& newQNH) {
	if (workerThread.joinable() || (openAIPinputFiles.empty() && openAirInputFiles.empty())) return false;
	Altitude::SetQNH(newQNH);
	//abort = false;
	workerThread = std::thread(std::bind(&Processor::LoadAirspacesfilesThread, this));
	return true;
}

void Processor::LoadAirspacesfilesThread() {
	for (const std::string& inputFile : openAIPinputFiles) OpenAIPreader::ReadFile(inputFile, airspaces);
	openAIPinputFiles.clear();
	OpenAir openAir(airspaces);
	for (const std::string& inputFile : openAirInputFiles) openAir.ReadFile(inputFile);
	openAirInputFiles.clear();
	PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}

bool Processor::UnloadAirspaces() {
	if (workerThread.joinable()) return false;
	airspaces.clear();
	return true;
}

bool Processor::LoadDEMfiles() {
	if (workerThread.joinable() || DEMfiles.empty()) return false;
	//abort = false;
	workerThread = std::thread(std::bind(&Processor::LoadDEMfilesThread, this));
	return true;
}

void Processor::LoadDEMfilesThread() {
	for (const std::string& demFile : DEMfiles) KMLwriter::AddTerrainMap(demFile);
	DEMfiles.clear();
	PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}

bool Processor::UnloadRasterMaps() {
	if (workerThread.joinable()) return false;
	KMLwriter::ClearTerrainMaps();
	return true;
}

bool Processor::LoadWaypointsFiles() {
	if (workerThread.joinable() || CUPfiles.empty()) return false;
	//abort = false;
	workerThread = std::thread(std::bind(&Processor::LoadWaypointsFilesThread, this));
	return true;
}

void Processor::LoadWaypointsFilesThread() {
	for (const std::string& inputFile : CUPfiles) CUPreader::ReadFile(inputFile, waypoints);
	CUPfiles.clear();
	PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}

bool Processor::UnloadWaypoints() {
	if (workerThread.joinable()) return false;
	for (const std::pair<const int, Waypoint*>& wpt : waypoints) delete wpt.second;
	waypoints.clear();
	return true;
}

bool Processor::MakeKMZfile(const std::string& outputKMZfile, const double& defaultTerraninAltMt)
{
	if (workerThread.joinable()) return false;
	outputFile = outputKMZfile;
	KMLwriter::SetDefaultTerrainAltitude(defaultTerraninAltMt);
	//abort = false;
	workerThread = std::thread(std::bind(&Processor::MakeKMZfileThread, this));
	return true;
}

void Processor::MakeKMZfileThread()
{
	KMLwriter writer;
	if (writer.WriteFile(outputFile, airspaces, waypoints)) PostMessage(window, writer.WereAllAGLaltitudesCovered() ? WM_WRITE_OUTPUT_OK : WM_WRITE_KML_AGL_WARNING, 0, 0);
	else PostMessage(window, WM_GENERAL_WORK_DONE, 0, 0);
}

bool Processor::MakeOtherFile(const std::string& outputFilename, AirspaceConverter::OutputType type)
{
	if (workerThread.joinable()) return false;
	outputFile = outputFilename;
	outputType = type;
	//abort = false;
	workerThread = std::thread(std::bind(&Processor::MakeOtherFileThread, this));
	return true;
}

void Processor::MakeOtherFileThread()
{
	bool done = false;
	switch (outputType) {
	case AirspaceConverter::OpenAir:
		done = OpenAir(airspaces).WriteFile(outputFile);
		break;
	case AirspaceConverter::Polish:
		done = PFMwriter().WriteFile(outputFile, airspaces);
		break;
	case AirspaceConverter::Garmin:
		{
			const std::string polishFile(boost::filesystem::path(outputFile).replace_extension(".mp").string());
			if(!PFMwriter().WriteFile(polishFile, airspaces)) break; // First make the Polish file
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
			if (lpShellExecInfo.hProcess == NULL) {
				AirspaceConverter::LogMessage("ERROR: Failed to start cGPSmapper process.", true);
				break;
			}
			WaitForSingleObject(lpShellExecInfo.hProcess, INFINITE);
			CloseHandle(lpShellExecInfo.hProcess);
			std::remove(polishFile.c_str()); // Delete polish file
			done = true;
		}
		break;
	default:
		assert(false);
		break;
	}
	PostMessage(window, done ? WM_WRITE_OUTPUT_OK : WM_GENERAL_WORK_DONE, 0, 0);
}

int Processor::GetNumOfTerrainMaps() const
{
	return KMLwriter::GetNumOfRasterMaps();
}
