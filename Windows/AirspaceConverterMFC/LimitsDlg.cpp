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

CLimitsDlg::CLimitsDlg():
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

BEGIN_MESSAGE_MAP(CLimitsDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_UNLIMITED_ALT_LIMIT, &CLimitsDlg::OnBnClickedCheckUnlimitedAltLimit)
	ON_BN_CLICKED(IDC_CHECK_FILTER_ON_ALTITUDE, &CLimitsDlg::OnBnClickedCheckFilterOnAltitude)
	ON_BN_CLICKED(IDC_CHECK_FILTER_ON_AREA, &CLimitsDlg::OnBnClickedCheckFilterOnArea)
END_MESSAGE_MAP()

BOOL CLimitsDlg::OnInitDialog() {
	CDialog::OnInitDialog();
	topAltitudeUnitCombo.InsertString(-1, _T("ft"));
	topAltitudeUnitCombo.InsertString(-1, _T("m"));
	topAltitudeUnitCombo.SetCurSel(0);
	lowAltitudeUnitCombo.InsertString(-1, _T("ft"));
	lowAltitudeUnitCombo.InsertString(-1, _T("m"));
	lowAltitudeUnitCombo.SetCurSel(0);
	return TRUE;
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
	double topAltLimit = topAltitude.GetAltFt();
	DDX_Text(pDX, IDC_TOP_ALT_LIMIT, topAltLimit);
	DDV_MinMaxDouble(pDX, topAltLimit, -10000, 200000);
	if (!topAltitude.IsUnlimited()) {
		int topAltUnitIndex = 0; // Index 0 default feet
		DDX_CBIndex(pDX, IDC_COMBO_TOP_ALT_UNIT, topAltUnitIndex);
		topAltitude = Altitude(topAltLimit, topAltUnitIndex == 1, true);
	}
	double lowAltLimit = lowAltitude.GetAltFt();
	DDX_Text(pDX, IDC_LOW_ALT_LIMIT, lowAltLimit);
	DDV_MinMaxDouble(pDX, lowAltLimit, -10000, 200000);
	int lowAltUnitIndex = 0; // Index 0 default feet
	DDX_CBIndex(pDX, IDC_COMBO_LOW_ALT_UNIT, lowAltUnitIndex);
	lowAltitude = Altitude(lowAltLimit, lowAltUnitIndex == 1, true);
	DDX_Control(pDX, IDC_CHECK_FILTER_ON_AREA, filterOnAreaCheckbox);
	DDX_Control(pDX, IDC_CHECK_FILTER_ON_ALTITUDE, filterOnAltitudeCheckbox);
	DDX_Control(pDX, IDC_CHECK_UNLIMITED_ALT_LIMIT, unlimitedTopAltitudeCheckbox);
	DDX_Control(pDX, IDC_COMBO_TOP_ALT_UNIT, topAltitudeUnitCombo);
	DDX_Control(pDX, IDC_COMBO_LOW_ALT_UNIT, lowAltitudeUnitCombo);
	DDX_Control(pDX, IDC_NORTH, northLatLimitEdit);
	DDX_Control(pDX, IDC_SOUTH, southLatLimitEdit);
	DDX_Control(pDX, IDC_EAST, eastLonLimitEdit);
	DDX_Control(pDX, IDC_WEST, westLonLimitEdit);
	DDX_Control(pDX, IDC_TOP_ALT_LIMIT, topAltitudeEdit);
	DDX_Control(pDX, IDC_LOW_ALT_LIMIT, lowAltitudeEdit);
}

void CLimitsDlg::OnOK() {
	if (!UpdateData(TRUE)) return;
	bool validAreaLimits = true, validAltitudeLimits = true;
	if (filterOnAreaCheckbox.GetCheck() == BST_CHECKED) {
		validAreaLimits = Geometry::Limits(northLatLimit, southLatLimit, westLonLimit, eastLonLimit).IsValid();
		validAreaLimitsSet = validAreaLimits;
	} else validAreaLimitsSet = false;
	if (filterOnAltitudeCheckbox.GetCheck() == BST_CHECKED) {
		validAltitudeLimits = (topAltitude >= lowAltitude);
		validAltitudeLimitsSet = validAltitudeLimits;
	} else validAltitudeLimitsSet = false;
	if (validAreaLimits && validAltitudeLimits) return CDialog::OnOK();
	if (!validAreaLimits) MessageBox(_T("The inserted area limits are not valid!"), _T("Error"), MB_ICONERROR);
	if (!validAltitudeLimits) MessageBox(_T("The inserted altitude limits are not valid!"), _T("Error"), MB_ICONERROR);
}

void CLimitsDlg::OnBnClickedCheckUnlimitedAltLimit() {
	const bool topLimited(unlimitedTopAltitudeCheckbox.GetCheck() != BST_CHECKED);
	topAltitude.SetUnlimited(!topLimited);
	topAltitudeEdit.EnableWindow(topLimited);
	topAltitudeUnitCombo.EnableWindow(topLimited);
}

void CLimitsDlg::OnBnClickedCheckFilterOnAltitude() {
	const bool filterOnAltitude(filterOnAltitudeCheckbox.GetCheck() == BST_CHECKED);
	unlimitedTopAltitudeCheckbox.EnableWindow(filterOnAltitude);
	const bool topLimited = (unlimitedTopAltitudeCheckbox.GetCheck() != BST_CHECKED);
	topAltitudeEdit.EnableWindow(filterOnAltitude && topLimited);
	topAltitudeUnitCombo.EnableWindow(filterOnAltitude && topLimited);
	lowAltitudeEdit.EnableWindow(filterOnAltitude);
	lowAltitudeUnitCombo.EnableWindow(filterOnAltitude);
}

void CLimitsDlg::OnBnClickedCheckFilterOnArea() {
	const bool filterOnArea(filterOnAreaCheckbox.GetCheck() == BST_CHECKED);
	northLatLimitEdit.EnableWindow(filterOnArea);
	southLatLimitEdit.EnableWindow(filterOnArea);
	eastLonLimitEdit.EnableWindow(filterOnArea);
	westLonLimitEdit.EnableWindow(filterOnArea);
}
