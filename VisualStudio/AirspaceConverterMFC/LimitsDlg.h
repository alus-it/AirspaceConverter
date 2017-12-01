//============================================================================
// AirspaceConverter
// Since       : 1/12/2017
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2017 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include "afxwin.h"

// CLimitsDlg dialog used to se the limits for filtering
class CLimitsDlg : public CDialog {
public:
	CLimitsDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIMITS_DIALOG };
#endif

	inline bool HasValidLimits() const { return validLimitsSet; }
	inline double GetTopLatLimit() const { return northLatLimit; }
	inline double GetBottomLatLimit() const { return southLatLimit; }
	inline double GetLeftLonLimit() const { return westLonLimit; }
	inline double GetRightLonLimit() const { return eastLonLimit; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

private:
	double northLatLimit;
	double southLatLimit;
	double eastLonLimit;
	double westLonLimit;
	bool validLimitsSet;
};
