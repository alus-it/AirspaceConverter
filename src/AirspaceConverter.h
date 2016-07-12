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

#ifndef VERSION
#define VERSION "0.0.0.9"
#endif

#include <functional>

class AirspaceConverter
{

public:
	static std::function<void(const std::string&, const bool)> LogMessage;
	inline static void SetLogMessageFuntion(std::function<void(const std::string&, const bool)> func) { LogMessage = func; }
	

private:
	static void DefaultLogMessage(const std::string&, const bool isError = false);

};
