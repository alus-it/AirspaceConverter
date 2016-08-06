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

#include "stdafx.h"
#include "Processor.h"
#include "../../src/AirspaceConverter.h"
#include "../../src/OpenAIPreader.h"
#include "../../src/Airspace.h"
#include "../../src/KMLwriter.h"
#include "../../src/PFMwriter.h"
#include "../../src/OpenAir.h"
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

bool Processor::MakeKMLfile(const std::string& outputKMLfile, const double& defaultTerraninAltMt)
{
	if (workerThread.joinable()) return false;
	outputFile = outputKMLfile;
	KMLwriter::SetDefaultTerrainAltitude(defaultTerraninAltMt);
	//abort = false;
	workerThread = std::thread(std::bind(&Processor::MakeKMLfileThread, this));
	return true;
}

void Processor::MakeKMLfileThread()
{
	KMLwriter writer;
	if (writer.WriteFile(outputFile, airspaces)) PostMessage(window, writer.WereAllAGLaltitudesCovered() ? WM_WRITE_OUTPUT_OK : WM_WRITE_KML_AGL_WARNING, 0, 0);
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
		{
			OpenAir writer(airspaces);
			done = writer.WriteFile(outputFile);
		}
		break;
	case AirspaceConverter::Polish:
		{
			PFMwriter writer;
			done = writer.WriteFile(outputFile, airspaces);
		}
		break;
	case AirspaceConverter::Garmin:
		{
			boost::filesystem::path polishPath(outputFile);
			polishPath.replace_extension(".mp");
			const std::string polishFile(polishPath.string());
			PFMwriter writer;
			if(!writer.WriteFile(polishFile, airspaces)) break; // First make the Polish file
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
