//============================================================================
// AirspaceConverter
// Since       : 1/12/2017
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2017 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "stdafx.h"
#include "LimitsDlg.hpp"
#include "Resource.h"
#include "Geometry.hpp"

CLimitsDlg::CLimitsDlg() :
	CDialog(IDD_LIMITS_DIALOG),
	northLatLimit(90),
	southLatLimit(-90),
	eastLonLimit(180),
	westLonLimit(-180),
	validAreaLimitsSet(false),
	topAltitude(200000.0),
	lowAltitude(-10000.0),
	validAltitudeLimitsSet(false) {
}

void CLimitsDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NORTH, northLatLimit);
	DDV_MinMaxDouble(pDX, northLatLimit, -90, 90);
	DDX_Text(pDX, IDC_SOUTH, southLatLimit);
	DDV_MinMaxDouble(pDX, southLatLimit, -90, 90);
	DDX_Text(pDX, IDC_EAST, eastLonLimit);
	DDV_MinMaxDouble(pDX, eastLonLimit, -180, 180);
	DDX_Text(pDX, IDC_WEST, westLonLimit);
	DDV_MinMaxDouble(pDX, westLonLimit, -180, 180);

	//TODO: work in progress...
	double topAltLimit = topAltitude.GetAltFt();
	DDX_Text(pDX, IDC_TOP_ALT_LIMIT, topAltLimit);
	DDV_MinMaxDouble(pDX, topAltLimit, -10000, 200000);
	topAltitude = Altitude(topAltLimit);
	double lowAltLimit = lowAltitude.GetAltFt();
	DDX_Text(pDX, IDC_LOW_ALT_LIMIT, lowAltLimit);
	DDV_MinMaxDouble(pDX, lowAltLimit, -10000, 200000);
	lowAltitude = Altitude(lowAltLimit);
}

void CLimitsDlg::OnOK() {
	if (!UpdateData(TRUE)) return;
	validAreaLimitsSet = Geometry::Limits(northLatLimit, southLatLimit, westLonLimit, eastLonLimit).IsValid();
	if (validAreaLimitsSet) return CDialog::OnOK();
	MessageBox(_T("The inserted limits are not valid!"), _T("Error"), MB_ICONERROR);
}
