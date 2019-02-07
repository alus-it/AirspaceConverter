//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <string>
#include <chrono>

class AirspaceConverter;
class Processor;

class CAirspaceConverterDlg : public CDialog {
public:
	CAirspaceConverterDlg(CWnd* pParent = nullptr);
	~CAirspaceConverterDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AIRSPACECONVERTER_DIALOG };
#endif

private:
	virtual void DoDataExchange(CDataExchange* pDX);
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedAbout();
	afx_msg void OnBnClickedInputFile();
	afx_msg void OnBnClickedInputWaypoints();
	afx_msg void OnBnClickedLoadDEM();
	afx_msg void OnBnClickedInputFolderBt();
	afx_msg void OnBnClickedInputWaypointsFolderBt();
	afx_msg void OnBnClickedLoadDemFolderBt();
	afx_msg void OnBnClickedClearInputBt();
	afx_msg void OnBnClickedClearWaypointsBt();
	afx_msg void OnBnClickedClearMapsBt();
	afx_msg void OnBnClickedFilterBt();
	afx_msg void OnBnClickedClearLogBt();
	afx_msg void OnBnClickedConvert();
	afx_msg void OnBnClickedOpenOutputFile();
	afx_msg void OnBnClickedOpenOutputFolder();
	afx_msg void OnBnClickedOutputTypeCombo();
	LRESULT OnEndJob(WPARAM, LPARAM);

	void LogMessage(const std::string& text, const bool isError = false);
	void UpdateOutputFilename();
	void EndBusy(const bool takeTime = false);
	void StartBusy();
	bool BrowseForFolderDialog(std::string& path) const;
	
	CComboBox OutputTypeCombo;
	CButton loadInputFileBt;
	CButton loadDEMfileBt;
	CButton ConvertBt;
	CButton OpenOutputFileBt;
	CButton OpenOutputFolderBt;
	CButton LoadAirspacesFolderBt;
	CButton LoadRasterMapsFolderBt;
	CButton loadWaypointsBt;
	CButton loadWaypointsFolderBt;
	CButton unloadAirspacesBt;
	CButton unloadRasterMapsBt;
	CButton unloadWaypointsBt;
	CButton filterBt;
	CButton ClearLogBt;
	CButton CloseButton;
	CEdit editQNHtextField;
	CEdit editDefualtAltTextField;
	CButton pointsCheckBox;
	CButton secondsCheckBox;
	CRichEditCtrl LoggingBox;
	CProgressCtrl progressBar;

	double QNH;
	double defaultTerrainAlt;
	std::string outputFile;
	bool busy;
	AirspaceConverter* converter;
	Processor* processor;
	unsigned long numAirspacesLoaded;
	unsigned long numWaypointsLoaded;
	int numRasterMapLoaded;
#ifndef _WIN64
	bool isWinXPorOlder;
#endif
	bool conversionDone;
	std::chrono::high_resolution_clock::time_point startTime;
};
