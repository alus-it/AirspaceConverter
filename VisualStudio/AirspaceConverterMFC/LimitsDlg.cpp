//============================================================================
// AirspaceConverter
// Since       : 1/12/2017
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2021 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "stdafx.h"
#include "LimitsDlg.h"
#include "Resource.h"
#include "Geometry.h"

CLimitsDlg::CLimitsDlg() :
	CDialog(IDD_LIMITS_DIALOG),
	northLatLimit(90),
	southLatLimit(-90),
	eastLonLimit(180),
	westLonLimit(-180),
	validLimitsSet(false) {
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
}

void CLimitsDlg::OnOK() {
	if (!UpdateData(TRUE)) return;
	validLimitsSet = Geometry::Limits(northLatLimit, southLatLimit, westLonLimit, eastLonLimit).IsValid();
	if (validLimitsSet) return CDialog::OnOK();
	MessageBox(_T("The inserted limits are not valid!"), _T("Error"), MB_ICONERROR);
}
