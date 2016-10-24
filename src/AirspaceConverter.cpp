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
#include <iostream>

std::function<void(const std::string&, const bool)> AirspaceConverter::LogMessage = DefaultLogMessage;

const std::vector<std::string> AirspaceConverter::disclaimer = {
	"Airspace file produced with: \"AirspaceConverter\" Version: " VERSION,
	"For info visit: http://www.alus.it/AirspaceConverter",
	"Copyrights(C) 2016 Alberto Realis-Luc",
	"",
	"WARNING:",
	"This is a BETA version of AirspaceConverter!",
	"So beware that the output may contain errors!!!",
	"The only usage of this software is to test it; so, you, as tester user, are kindly requested to report any error or discepancy found.",
	"",
	"Disclaimer:",
	"The author of AirspaceConverter assumes no liability at all for the previous, actual or future correctness, completeness, functionality or usability",
	"of the data provided in this file and the usage of AirspaceConverter. There exists no obligation at all for the author to continuously update",
	"or maintain the data provided. The airspace structure in this file and the data contained therein are only intended to serve as a means to facilitate",
	"familiarization with and to illustrate air space structure. This airspace structure file does not replace the pilot's obligation for preflight",
	"planning nor shall it be used as a means of support during flight. In particular, use of the this airspace structure file does not excuse the user",
	"from the responsibility to observe the current issue of any relevant AIP, AIP Supplements, NOTAM and AICs.",
	"The use of this airspace structure file takes place only at the user's total own risk.",
	"Commercial use of the data provided via this airspace structure file is strictly prohibited.",
	"The use of AirspaceConverter is only at complete user's own risk.",
	"Any commercial usage of AirspaceConverter is also strictly prohibited if not authorized by the author.",
	"",
	"Error reports, complaints and suggestions please email to: info@alus.it",
};

void AirspaceConverter::DefaultLogMessage(const std::string& msgText, const bool isError) {
	(isError ? std::cerr : std::cout) << msgText << std::endl;
}

std::istream& AirspaceConverter::safeGetline(std::istream& is, std::string& line, bool& isCRLF)
{
	line.clear();
	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();
	isCRLF = false;

	for(;;) {
		const int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return is;
		case '\r':
			if(sb->sgetc() == '\n') {
				sb->sbumpc();
				isCRLF = true;
			}
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if(line.empty()) {
				is.setstate(std::ios::eofbit);
				isCRLF = true; // no problem in this case
			}
			return is;
		default:
			line += (char)c;
		}
	}
}
