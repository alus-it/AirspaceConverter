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

#include "AirspaceConverter.h"
#include <string>
#include <iostream>

std::function<void(const std::string&, const bool)> AirspaceConverter::LogMessage = DefaultLogMessage;

void AirspaceConverter::DefaultLogMessage(const std::string& msgText, const bool isError)
{
	(isError ? std::cerr : std::cout) << msgText << std::endl;
}
