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

#pragma once
#include "afxwin.h"
#include "Altitude.hpp"

class CLimitsDlg : public CDialog {
public:
	CLimitsDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIMITS_DIALOG };
#endif

	inline double GetTopLatLimit() const { return northLatLimit; }
	inline double GetBottomLatLimit() const { return southLatLimit; }
	inline double GetLeftLonLimit() const { return westLonLimit; }
	inline double GetRightLonLimit() const { return eastLonLimit; }
	inline bool HasValidAreaLimits() const { return validAreaLimitsSet; }
	inline const Altitude& GetTopAltitude() const { return topAltitude; }
	inline const Altitude& GetLowAltitude() const { return lowAltitude; }
	inline bool HasValidAltitudeLimits() const { return validAltitudeLimitsSet; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

private:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCheckUnlimitedAltLimit();
	afx_msg void OnBnClickedCheckFilterOnAltitude();
	afx_msg void OnBnClickedCheckFilterOnArea();

	double northLatLimit;
	double southLatLimit;
	double eastLonLimit;
	double westLonLimit;
	bool validAreaLimitsSet;
	Altitude topAltitude;
	Altitude lowAltitude;
	bool validAltitudeLimitsSet;
	CButton filterOnAreaCheckbox;
	CButton filterOnAltitudeCheckbox;
	CButton unlimitedTopAltitudeCheckbox;
	CComboBox topAltitudeUnitCombo;
	CComboBox lowAltitudeUnitCombo;
	CEdit northLatLimitEdit;
	CEdit southLatLimitEdit;
	CEdit eastLonLimitEdit;
	CEdit westLonLimitEdit;
	CEdit topAltitudeEdit;
	CEdit lowAltitudeEdit;
};
