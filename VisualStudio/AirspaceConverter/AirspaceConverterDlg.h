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

class CAirspaceConverterDlg : public CDialog
{
public:
	CAirspaceConverterDlg(CWnd* pParent = NULL);
	~CAirspaceConverterDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AIRSPACECONVERTER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
private:
	afx_msg void OnBnClickedAbout();
	afx_msg void OnBnClickedInputFile();
	afx_msg void OnBnClickedLoadDEM();
	afx_msg void OnBnClickedInputFolderBt();
	afx_msg void OnBnClickedLoadDemFolderBt();
	afx_msg void OnBnClickedClearInputBt();
	afx_msg void OnBnClickedClearMapsBt();
	afx_msg void OnBnClickedMakeKML();
	afx_msg void OnBnClickedCompress();
	afx_msg void OnBnClickedOpenOutputFile();
	afx_msg void OnBnClickedOpenOutputFolder();
	afx_msg void OnBnClickedChooseOutputFileBt();
	
	LRESULT OnGeneralEndOperations(WPARAM, LPARAM);
	LRESULT OnEndWriteKMLok(WPARAM, LPARAM);
	LRESULT OnEndWriteKMLwarningAGL(WPARAM, LPARAM);

	void UpdateOutputFilename();
	void LogMessage(const std::string& text, const bool isError = false);
	
	CButton loadInputFileBt;
	CButton loadDEMfileBt;
	CButton MakeKMLBt;
	CButton OpenOutputFileBt;
	CButton OpenOutputFolderBt;
	CButton LoadAirspacesFolderBt;
	CButton LoadRasterMapsFolderBt;
	CButton unloadAirspacesBt;
	CButton unloadRasterMapsBt;
	CButton compressCheckBox;
	CButton chooseOutputFileBt;
	CButton CloseButton;
	CEdit editQNHtextField;
	CEdit editDefualtAltTextField;
	CEdit outputFileEditBox;
	BOOL compress;
	CProgressCtrl progressBar;

	void EndBusy();
	void StartBusy();
	
	double QNH;
	double defaultTerrainAlt;
	std::string outputFile;
	bool busy;
	Processor* processor;
	unsigned long numAirspacesLoaded;
	int numRasterMapLoaded;
	bool isWinXPorOlder;
	bool conversionDone;
	CRichEditCtrl LoggingBox;
};
