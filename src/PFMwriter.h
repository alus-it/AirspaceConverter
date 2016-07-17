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
#include <map>
#include <fstream>

class Airspace;

class PFMwriter {
public:
	inline PFMwriter() {}
	inline ~PFMwriter() {}

	bool WriteFile(const std::string& filename, const std::multimap<int, Airspace>& airspaces);

private:
	void WriteHeader(const std::string& filename);
	const static std::string MakeLabel(const Airspace& airspace);

	//static const int types[];

	std::ofstream file;
};

