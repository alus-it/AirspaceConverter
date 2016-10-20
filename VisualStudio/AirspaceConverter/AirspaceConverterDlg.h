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
#include "afxcmn.h"
#include "afxwin.h"
#include <string>

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
	afx_msg void OnBnClickedConvert();
	afx_msg void OnBnClickedOpenOutputFile();
	afx_msg void OnBnClickedOpenOutputFolder();
	afx_msg void OnBnClickedChooseOutputFileBt();
	afx_msg void OnBnClickedOutputTypeCombo();
	
	LRESULT OnGeneralEndOperations(WPARAM, LPARAM);
	LRESULT OnEndWriteKMLok(WPARAM, LPARAM);
	LRESULT OnEndWriteKMLwarningAGL(WPARAM, LPARAM);

	void LogMessage(const std::string& text, const bool isError = false);
	void UpdateOutputFilename();
	void EndBusy();
	void StartBusy();
	
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
	CButton chooseOutputFileBt;
	CButton CloseButton;
	CEdit editQNHtextField;
	CEdit editDefualtAltTextField;
	CEdit outputFileEditBox;
	CRichEditCtrl LoggingBox;
	CProgressCtrl progressBar;

	double QNH;
	double defaultTerrainAlt;
	std::string outputFile;
	bool busy;
	Processor* processor;
	unsigned long numAirspacesLoaded;
	unsigned long numWaypointsLoaded;
	int numRasterMapLoaded;
	bool isWinXPorOlder;
	bool conversionDone;
};
