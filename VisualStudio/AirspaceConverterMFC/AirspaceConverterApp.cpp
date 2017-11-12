//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2017 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "stdafx.h"
#include "AirspaceConverterApp.h"
#include "AirspaceConverterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAirspaceConverterApp
BEGIN_MESSAGE_MAP(CAirspaceConverterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CAirspaceConverterApp::CAirspaceConverterApp() {
	// Add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CAirspaceConverterApp object
CAirspaceConverterApp theApp;

// CAirspaceConverterApp initialization
BOOL CAirspaceConverterApp::InitInstance() {
	CWinApp::InitInstance();

	// Call AfxInitRichEdit2() to initialize richedit2 library.
	AfxInitRichEdit();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Local AirspaceConverter Applications"));
	
	CAirspaceConverterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	/*if (nResponse == IDOK) {} //  dismissed with OK
	else if (nResponse == IDCANCEL) {} //  dismissed with Cancel
	else*/
	if (nResponse == -1) TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
	
	// Since the dialog has been closed, return FALSE so that we exit the application, rather than start the application's message pump.
	return FALSE;
}
