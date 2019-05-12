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

#include "stdafx.h"
#include "afxdialogex.h"
#include "afxwin.h"
#include "AirspaceConverterApp.h"
#include "AirspaceConverterDlg.h"
#include "LimitsDlg.h"
#include "Processor.h"
#include "AirspaceConverter.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/locale/encoding.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	afx_msg void OnNMClickLink(NMHDR *pNMHDR, LRESULT *pResult);
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX) {}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog() {
	if (!CDialog::OnInitDialog()) return FALSE;
	this->GetDlgItem(IDC_VERSION)->SetWindowTextW(_T(VERSION));
	std::string compileTime("Compiled on ");
	compileTime.append(__DATE__);
	compileTime.append(" at ");
	compileTime.append(__TIME__);
	this->GetDlgItem(IDC_COMPILE_TIME)->SetWindowTextW(CString(compileTime.c_str()));
	return TRUE;
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_WEB, &CAboutDlg::OnNMClickLink)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_EMAIL, &CAboutDlg::OnNMClickLink)
END_MESSAGE_MAP()

void CAboutDlg::OnNMClickLink(NMHDR* pNMHDR, LRESULT* pResult) {
	PNMLINK pNMLink = (PNMLINK)pNMHDR;
	ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
	*pResult = 0;
}

CAirspaceConverterDlg::CAirspaceConverterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_AIRSPACECONVERTER_DIALOG, pParent)
	, QNH(1013.25)
	, defaultTerrainAlt(50)
	, converter(nullptr)
	, processor(nullptr)
	, m_hIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME))
	, numAirspacesLoaded(0)
	, numWaypointsLoaded(0)
	, numRasterMapLoaded(0)
	, busy(false)
#ifndef _WIN64
	, isWinXPorOlder(false)
#endif
	, conversionDone(false) {
}

CAirspaceConverterDlg::~CAirspaceConverterDlg() {
	if (converter != nullptr) delete converter;
	if (processor != nullptr) delete processor;
}

void CAirspaceConverterDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NUM_AIRSPACES, numAirspacesLoaded);
	DDX_Text(pDX, IDC_NUM_WAYPOINTS, numWaypointsLoaded);
	DDX_Text(pDX, IDC_NUM_RASTER_MAPS, numRasterMapLoaded);
	DDX_Text(pDX, IDC_QNH_EDIT, QNH);
	DDV_MinMaxDouble(pDX, QNH, 800, 1050);
	DDX_Text(pDX, IDC_DEFAULT_TERRAIN_ALT_EDIT, defaultTerrainAlt);
	DDV_MinMaxDouble(pDX, defaultTerrainAlt, -100, 10000);
	DDX_Control(pDX, IDC_PROGRESS_BAR, progressBar);
	DDX_Control(pDX, IDC_INPUT_FILE_BT, loadInputFileBt);
	DDX_Control(pDX, IDC_INPUT_WAYPOINTS_BT, loadWaypointsBt);
	DDX_Control(pDX, IDC_LOAD_DEM_BT, loadDEMfileBt);
	DDX_Control(pDX, IDC_CONVERT_BT, ConvertBt);
	DDX_Control(pDX, IDC_OPEN_OUTPUT_FILE, OpenOutputFileBt);
	DDX_Control(pDX, IDC_OPEN_OUTPUT_FOLDER, OpenOutputFolderBt);
	DDX_Control(pDX, IDOK, CloseButton);
	DDX_Control(pDX, IDC_INPUT_FOLDER_BT, LoadAirspacesFolderBt);
	DDX_Control(pDX, IDC_LOAD_DEM_FOLDER_BT, LoadRasterMapsFolderBt);
	DDX_Control(pDX, IDC_INPUT_WAYPOINTS_FOLDER_BT, loadWaypointsFolderBt);
	DDX_Control(pDX, IDC_QNH_EDIT, editQNHtextField);
	DDX_Control(pDX, IDC_DEFAULT_TERRAIN_ALT_EDIT, editDefualtAltTextField);
	DDX_Control(pDX, IDC_CLEAR_INPUT_BT, unloadAirspacesBt);
	DDX_Control(pDX, IDC_CLEAR_MAPS_BT, unloadRasterMapsBt);
	DDX_Control(pDX, IDC_CLEAR_WAYPOINTS_BT, unloadWaypointsBt);
	DDX_Control(pDX, IDC_FILTER_BT, filterBt);
	DDX_Control(pDX, IDC_CLEAR_LOG_BT, ClearLogBt);
	DDX_Control(pDX, IDC_LOG, LoggingBox);
	DDX_Control(pDX, IDC_COMBO_OUTPUT_TYPE, OutputTypeCombo);
	DDX_Control(pDX, IDC_CHECK_POINTS, pointsCheckBox);
	DDX_Control(pDX, IDC_CHECK_SECONDS, secondsCheckBox);
}

BEGIN_MESSAGE_MAP(CAirspaceConverterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_INPUT_FILE_BT, &CAirspaceConverterDlg::OnBnClickedInputFile)
	ON_BN_CLICKED(IDC_INPUT_WAYPOINTS_BT, &CAirspaceConverterDlg::OnBnClickedInputWaypoints)
	ON_BN_CLICKED(IDC_LOAD_DEM_BT, &CAirspaceConverterDlg::OnBnClickedLoadDEM)
	ON_BN_CLICKED(IDC_CONVERT_BT, &CAirspaceConverterDlg::OnBnClickedConvert)
	ON_MESSAGE(WM_GENERAL_WORK_DONE, &CAirspaceConverterDlg::OnEndJob)
	ON_BN_CLICKED(IDC_OPEN_OUTPUT_FILE, &CAirspaceConverterDlg::OnBnClickedOpenOutputFile)
	ON_BN_CLICKED(IDC_OPEN_OUTPUT_FOLDER, &CAirspaceConverterDlg::OnBnClickedOpenOutputFolder)
	ON_BN_CLICKED(IDC_ABOUT, &CAirspaceConverterDlg::OnBnClickedAbout)
	ON_BN_CLICKED(IDC_INPUT_FOLDER_BT, &CAirspaceConverterDlg::OnBnClickedInputFolderBt)
	ON_BN_CLICKED(IDC_INPUT_WAYPOINTS_FOLDER_BT, &CAirspaceConverterDlg::OnBnClickedInputWaypointsFolderBt)
	ON_BN_CLICKED(IDC_LOAD_DEM_FOLDER_BT, &CAirspaceConverterDlg::OnBnClickedLoadDemFolderBt)
	ON_BN_CLICKED(IDC_CLEAR_INPUT_BT, &CAirspaceConverterDlg::OnBnClickedClearInputBt)
	ON_BN_CLICKED(IDC_CLEAR_WAYPOINTS_BT, &CAirspaceConverterDlg::OnBnClickedClearWaypointsBt)
	ON_BN_CLICKED(IDC_CLEAR_MAPS_BT, &CAirspaceConverterDlg::OnBnClickedClearMapsBt)
	ON_BN_CLICKED(IDC_FILTER_BT, &CAirspaceConverterDlg::OnBnClickedFilterBt)
	ON_BN_CLICKED(IDC_CLEAR_LOG_BT, &CAirspaceConverterDlg::OnBnClickedClearLogBt)
	ON_CBN_SELCHANGE(IDC_COMBO_OUTPUT_TYPE, &CAirspaceConverterDlg::OnBnClickedOutputTypeCombo)
END_MESSAGE_MAP()

BOOL CAirspaceConverterDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu. IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		assert(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Add extra initialization here ...

	// Initialize output type combo box
	OutputTypeCombo.InsertString(-1, _T("KMZ Google Earth"));
	OutputTypeCombo.InsertString(-1, _T("OpenAir (airspace only)"));
	OutputTypeCombo.InsertString(-1, _T("CUP SeeYou (waypoints only)"));
	OutputTypeCombo.InsertString(-1, _T("Polish format for cGPSmapper"));
	OutputTypeCombo.InsertString(-1, _T("IMG file for Garmin devices"));
	OutputTypeCombo.SetCurSel(AirspaceConverter::OutputType::KMZ_Format);

	// Check if is running on Windows XP (v 5.2) or older. Only on the 32 bit version, on 64 bit we assume that we are using something newer than WinXP
#ifndef _WIN64
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO*)&osvi) && osvi.dwMajorVersion * 10 + osvi.dwMinorVersion <= 52) isWinXPorOlder = true;
	
	// In case of Windows XP disable button that can't be used
	if (isWinXPorOlder) {
		OpenOutputFileBt.EnableWindow(FALSE);
		OpenOutputFolderBt.EnableWindow(FALSE);
	}
#endif

	// Initialize progress bar, necessary to do that that here to make it work properly also on WindowsXP
	progressBar.SetPos(0);
	progressBar.SetMarquee(FALSE, 1);
	progressBar.ModifyStyle(PBS_MARQUEE, 0); // MARQUEE set here, not in the resource file

	// Set the logging function (to write in the logging texbox... and not on the default Linux console)
	AirspaceConverter::SetLogMessageFunction(std::function<void(const std::string&)>(std::bind(&CAirspaceConverterDlg::LogMessage, this, std::placeholders::_1)));
	AirspaceConverter::SetLogWarningFunction(std::function<void(const std::string&)>(std::bind(&CAirspaceConverterDlg::LogWarning, this, std::placeholders::_1)));
	AirspaceConverter::SetLogErrorFunction(std::function<void(const std::string&)>(std::bind(&CAirspaceConverterDlg::LogError, this, std::placeholders::_1)));

	// Buld the "converter"
	converter = new AirspaceConverter();
	if (converter == nullptr) {
		MessageBox(_T("Fatal error while initilizing the converter utility."), _T("Error"), MB_ICONERROR);
		return TRUE;
	}

	// Find the path where the current executable is running
	TCHAR szPath[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetApp()->m_hInstance, szPath, _MAX_PATH));
	CString csPath(szPath);
	const int nIndex(csPath.ReverseFind(_T('\\')));
	if (nIndex > 0) csPath = csPath.Left(nIndex);
	else csPath.Empty();
	const std::string basePath = CT2CA(csPath);
	
	// Configure the paths to icons and to cGPSmapper
	assert(!basePath.empty());
	converter->SetIconsPath(std::string(basePath + "\\icons\\"));
	converter->Set_cGPSmapperCommand('"' + basePath + "\\cGPSmapper\\cgpsmapper.exe\"");

	// Buld the "processor"
	processor = new Processor(this->GetSafeHwnd(), converter);	
	if (processor == nullptr) MessageBox(_T("Fatal error while initilizing the application processing thread."), _T("Error"), MB_ICONERROR);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAirspaceConverterDlg::OnSysCommand(UINT nID, LPARAM lParam) {
	if (nID == SC_CLOSE && busy &&
		MessageBox(_T("The conversion is still in progress...\nThe application will remain anyway working in background.\nAre you really sure you want to exit now?"), _T("Exit"), MB_YESNO | MB_ICONWARNING) != IDYES)
		return;
	else if ((nID & 0xFFF0) == IDM_ABOUTBOX) OnBnClickedAbout();
	CDialog::OnSysCommand(nID, lParam);
}

void CAirspaceConverterDlg::OnBnClickedAbout() {
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CAirspaceConverterDlg::OnPaint() {
	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		CRect rect;
		GetClientRect(&rect); // Get client rectangle
		dc.DrawIcon((rect.Width() - GetSystemMetrics(SM_CXICON) + 1) / 2, (rect.Height() - GetSystemMetrics(SM_CYICON) + 1) / 2, m_hIcon); // Draw the icon centred in client rectangle
	} else CDialog::OnPaint();	
}

// The system calls this function to obtain the cursor to display while the user drags the minimized window.
HCURSOR CAirspaceConverterDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}

int CALLBACK BrowseForFolder(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	if (uMsg == BFFM_INITIALIZED) SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
	else if (lParam == 0) {}
	return 0;
}

bool CAirspaceConverterDlg::BrowseForFolderDialog(std::string& path) const {
	bool result(false);
	LPMALLOC pMalloc(nullptr);
	if (SHGetMalloc(&pMalloc) == NOERROR) {
		BROWSEINFO bi;
		ZeroMemory(&bi, sizeof(bi));
		CString sPath("C:\\");
		LPWSTR psFolder = sPath.GetBuffer(MAX_PATH);
		bi.hwndOwner = GetSafeHwnd();
		bi.pszDisplayName = psFolder;
		bi.lpszTitle = L"Please select desired input folder";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;
		bi.lpfn = BrowseForFolder;
		bi.lParam = (LPARAM)psFolder;
		bi.iImage = 0;
		LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);
		if (pidl != nullptr) {
			result = SHGetPathFromIDList(pidl, psFolder);
			pMalloc->Free(pidl);
		}
		sPath.ReleaseBuffer();
		path = CT2CA(sPath);
		pMalloc->Release();
	}
	return result;
}

void CAirspaceConverterDlg::StartBusy() {
	busy = true;
	progressBar.ModifyStyle(0, PBS_MARQUEE);
	progressBar.SetMarquee(TRUE, 1);
	loadInputFileBt.EnableWindow(FALSE);
	loadWaypointsBt.EnableWindow(FALSE);
	loadDEMfileBt.EnableWindow(FALSE);
	LoadAirspacesFolderBt.EnableWindow(FALSE);
	loadWaypointsFolderBt.EnableWindow(FALSE);
	LoadRasterMapsFolderBt.EnableWindow(FALSE);
#ifndef _WIN64
	if (!isWinXPorOlder) {
#endif
		OpenOutputFileBt.EnableWindow(FALSE);
		OpenOutputFolderBt.EnableWindow(FALSE);
#ifndef _WIN64
	}
#endif
	unloadAirspacesBt.EnableWindow(FALSE);
	unloadWaypointsBt.EnableWindow(FALSE);
	unloadRasterMapsBt.EnableWindow(FALSE);
	filterBt.EnableWindow(FALSE);
	OutputTypeCombo.EnableWindow(FALSE);
	editQNHtextField.EnableWindow(FALSE);
	editDefualtAltTextField.EnableWindow(FALSE);
	pointsCheckBox.EnableWindow(FALSE);
	secondsCheckBox.EnableWindow(FALSE);
	ClearLogBt.EnableWindow(FALSE);
	CloseButton.EnableWindow(FALSE);

	// Start the timer
	startTime = std::chrono::high_resolution_clock::now();
}

LRESULT CAirspaceConverterDlg::OnEndJob(WPARAM, LPARAM) {
	conversionDone = converter->IsConversionDone();
	EndBusy(true);
	return LRESULT();
}

void CAirspaceConverterDlg::UpdateOutputFilename() {
	conversionDone = false;
	if (!AirspaceConverter::PutTypeExtension((AirspaceConverter::OutputType)OutputTypeCombo.GetCurSel(), outputFile)) outputFile.clear();
}

void CAirspaceConverterDlg::LogMessage(const std::string& text) {
	CString message(CA2T((boost::locale::conv::between(text, "ISO8859-1", "utf-8") + '\n').c_str()));
	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = (DWORD)~CFE_AUTOCOLOR;
	cf.crTextColor = RGB(0, 0, 0);
	LoggingBox.SetSel(-1, -1); // Set the cursor to the end of the text area and deselect everything.
	LoggingBox.SetSelectionCharFormat(cf);
	LoggingBox.ReplaceSel(message); // Inserts when nothing is selected.
	int linesToScroll = LoggingBox.GetLineCount() - LoggingBox.GetFirstVisibleLine() - 13;
	if (linesToScroll > 0) LoggingBox.LineScroll(linesToScroll);
}

void CAirspaceConverterDlg::LogWarning(const std::string& text) {
	CString message(CA2T((boost::locale::conv::between("Warning: " + text, "ISO8859-1", "utf-8") + '\n').c_str()));
	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = (DWORD)~CFE_AUTOCOLOR;
	cf.crTextColor = RGB(255, 204, 0);
	LoggingBox.SetSel(-1, -1); // Set the cursor to the end of the text area and deselect everything.
	LoggingBox.SetSelectionCharFormat(cf);
	LoggingBox.ReplaceSel(message); // Inserts when nothing is selected.
	int linesToScroll = LoggingBox.GetLineCount() - LoggingBox.GetFirstVisibleLine() - 13;
	if (linesToScroll > 0) LoggingBox.LineScroll(linesToScroll);
}

void CAirspaceConverterDlg::LogError(const std::string& text) {
	CString message(CA2T((boost::locale::conv::between("ERROR: " + text, "ISO8859-1", "utf-8") + '\n').c_str()));
	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_BOLD | CFM_COLOR;
	cf.dwEffects = (DWORD)CFE_BOLD | ~CFE_AUTOCOLOR;
	cf.crTextColor = RGB(255, 0, 0);
	LoggingBox.SetSel(-1, -1); // Set the cursor to the end of the text area and deselect everything.
	LoggingBox.SetSelectionCharFormat(cf);
	LoggingBox.ReplaceSel(message); // Inserts when nothing is selected.
	int linesToScroll = LoggingBox.GetLineCount() - LoggingBox.GetFirstVisibleLine() - 13;
	if (linesToScroll > 0) LoggingBox.LineScroll(linesToScroll);
}

void CAirspaceConverterDlg::OnBnClickedClearLogBt() {
	LoggingBox.SetWindowTextW(_T(""));
}

void CAirspaceConverterDlg::EndBusy(const bool takeTime /* = false */) {
	assert(converter != nullptr);
	assert(processor != nullptr);
	if (processor != nullptr) processor->Join();
	if (takeTime) {
		const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
		LogMessage(std::string(boost::str(boost::format("Execution time: %1f sec.") % elapsedTimeSec)));
	}
	if(converter != nullptr) {
		numAirspacesLoaded = converter->GetNumOfAirspaces();
		numWaypointsLoaded = converter->GetNumOfWaypoints();
		numRasterMapLoaded = converter->GetNumOfTerrainMaps();
	} else {
		numAirspacesLoaded = 0;
		numRasterMapLoaded = 0;
	}
	const BOOL isKmzFile(OutputTypeCombo.GetCurSel() == AirspaceConverter::OutputType::KMZ_Format);
	const BOOL isOpenAirFile(OutputTypeCombo.GetCurSel() == AirspaceConverter::OutputType::OpenAir_Format);
	const BOOL isSeeYouFile(OutputTypeCombo.GetCurSel() == AirspaceConverter::OutputType::SeeYou_Format);
	loadInputFileBt.EnableWindow(!isSeeYouFile);
	loadWaypointsBt.EnableWindow(isKmzFile || isSeeYouFile);
	loadDEMfileBt.EnableWindow(isKmzFile);
	LoadAirspacesFolderBt.EnableWindow(!isSeeYouFile);
	loadWaypointsFolderBt.EnableWindow(isKmzFile || isSeeYouFile);
	LoadRasterMapsFolderBt.EnableWindow(isKmzFile);
#ifndef _WIN64
	if (!isWinXPorOlder) {
#endif
		OpenOutputFileBt.EnableWindow(conversionDone);
		OpenOutputFolderBt.EnableWindow(conversionDone);
#ifndef _WIN64
	}
#endif
	unloadAirspacesBt.EnableWindow(numAirspacesLoaded > 0 ? TRUE : FALSE);
	unloadWaypointsBt.EnableWindow(numWaypointsLoaded > 0 ? TRUE : FALSE);
	unloadRasterMapsBt.EnableWindow(numRasterMapLoaded > 0 ? TRUE : FALSE);
	filterBt.EnableWindow(numAirspacesLoaded > 0 ? TRUE : FALSE);
	ConvertBt.EnableWindow((numAirspacesLoaded > 0 || ((isKmzFile || isSeeYouFile) && numWaypointsLoaded > 0)) ? TRUE : FALSE);
	OutputTypeCombo.EnableWindow(TRUE);
	editQNHtextField.EnableWindow(isKmzFile ? numAirspacesLoaded == 0 : FALSE);
	editDefualtAltTextField.EnableWindow(isKmzFile);
	pointsCheckBox.EnableWindow(isOpenAirFile);
	secondsCheckBox.EnableWindow(isOpenAirFile);
	ClearLogBt.EnableWindow(TRUE);
	CloseButton.EnableWindow(TRUE);
	progressBar.SetMarquee(FALSE, 1);
	progressBar.ModifyStyle(PBS_MARQUEE, 0);
	progressBar.SetPos(0);
	UpdateData(FALSE);
	busy = false;
}

void CAirspaceConverterDlg::OnBnClickedInputFile() {
	assert(converter != nullptr);
	assert(processor != nullptr);
	if (!UpdateData(TRUE)) return; // Force the user to enter valid QNH
	CFileDialog dlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST, _T("All airspace files|*.txt; *.aip; *.kmz; *.kml|openAIP|*.aip|OpenAir|*.txt|Google Earth|*.kmz; *.kml||"), (CWnd*)this, 0, TRUE);
	if (dlg.DoModal() == IDOK) {
		outputFile.clear();
		conversionDone = false;
		POSITION pos(dlg.GetStartPosition());
		while (pos) {
			const std::string inputFilename(CT2CA(dlg.GetNextPathName(pos)));
			if (!boost::filesystem::is_regular_file(inputFilename)) continue;
			converter->AddAirspaceFile(inputFilename);
			if(outputFile.empty()) outputFile = inputFilename;
		}
		UpdateOutputFilename();
		if (processor != nullptr && processor->LoadAirspacesFiles(QNH)) StartBusy();
		else MessageBox(_T("Error while starting input thread."), _T("Error"), MB_ICONERROR);
	}
}

void CAirspaceConverterDlg::OnBnClickedInputWaypoints() {
	assert(converter != nullptr);
	assert(processor != nullptr);
	CFileDialog dlg(TRUE, _T("cup"), NULL, OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST, _T("SeeYou waypoints|*.cup||"), (CWnd*)this, 0, TRUE);
	if (dlg.DoModal() == IDOK) {
		POSITION pos(dlg.GetStartPosition());
		while (pos) {
			const std::string inputFilename(CT2CA(dlg.GetNextPathName(pos)));
			if (!boost::filesystem::is_regular_file(inputFilename)) continue;
			if(outputFile.empty()) outputFile = inputFilename;
			converter->AddWaypointFile(inputFilename);
		}
		UpdateOutputFilename();
		if (processor != nullptr && processor->LoadWaypointsFiles()) StartBusy();
		else MessageBox(_T("Error while starting read waypoints files thread."), _T("Error"), MB_ICONERROR);
	}
}

void CAirspaceConverterDlg::OnBnClickedLoadDEM() {
	assert(converter != nullptr);
	assert(processor != nullptr);
	CFileDialog dlg(TRUE, _T("dem"), NULL, OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST, _T("Terrain raster map|*.dem||"), (CWnd*)this, 0, TRUE);
	if (dlg.DoModal() == IDOK) {
		POSITION pos(dlg.GetStartPosition());
		while (pos) {
			const std::string inputFilename(CT2CA(dlg.GetNextPathName(pos)));
			if (!boost::filesystem::is_regular_file(inputFilename)) continue;
			converter->AddTerrainRasterMapFile(inputFilename);
		}
		if (processor != nullptr && processor->LoadDEMfiles()) StartBusy();
		else MessageBox(_T("Error while starting read raster maps thread."), _T("Error"), MB_ICONERROR);
	}
}

void CAirspaceConverterDlg::OnBnClickedInputFolderBt() {
	assert(converter != nullptr);
	assert(processor != nullptr);
	if (!UpdateData(TRUE)) return; // Force the user to enter valid QNH

	// Ask for the airspaces input folder
	std::string inputPath;
#ifndef _WIN64
	if (!isWinXPorOlder) {
#endif
		CFolderPickerDialog dlgFolder(NULL, OFN_PATHMUSTEXIST, (CWnd*)this);
		if (dlgFolder.DoModal() == IDOK) inputPath = CT2CA(dlgFolder.GetFolderPath());
		else return;
#ifndef _WIN64
	} else if (!BrowseForFolderDialog(inputPath)) return;
#endif
	assert(!inputPath.empty());

	outputFile.clear();
	conversionDone = false;
	boost::filesystem::path root(inputPath);
	if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen
	for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
		if (!boost::filesystem::is_regular_file(*it)) continue;
		converter->AddAirspaceFile(it->path().string());
		if (outputFile.empty()) outputFile = it->path().string();
	}
	UpdateOutputFilename();
	if (processor != nullptr && processor->LoadAirspacesFiles(QNH)) StartBusy();	
	else MessageBox(_T("Error while starting input thread."), _T("Error"), MB_ICONERROR);
}

void CAirspaceConverterDlg::OnBnClickedInputWaypointsFolderBt() {
	assert(converter != nullptr);
	assert(processor != nullptr);

	// Ask for the waypoints input folder
	std::string inputPath;
#ifndef _WIN64
	if (!isWinXPorOlder) {
#endif
		CFolderPickerDialog dlgFolder(NULL, OFN_PATHMUSTEXIST, (CWnd*)this);
		if (dlgFolder.DoModal() == IDOK) inputPath = CT2CA(dlgFolder.GetFolderPath());
		else return;
#ifndef _WIN64
	}
	else if (!BrowseForFolderDialog(inputPath)) return;
#endif
	assert(!inputPath.empty());

	boost::filesystem::path root(inputPath);
	if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen
	for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
		if (boost::filesystem::is_regular_file(*it) && boost::iequals(it->path().extension().string(), ".cup")) {
			converter->AddWaypointFile(it->path().string());
			if (outputFile.empty()) outputFile = it->path().string();
		}
	}
	UpdateOutputFilename();
	if (processor != nullptr && processor->LoadWaypointsFiles()) StartBusy();
	else MessageBox(_T("Error while starting read waypoints files thread."), _T("Error"), MB_ICONERROR);
}

void CAirspaceConverterDlg::OnBnClickedLoadDemFolderBt() {
	assert(converter != nullptr);
	assert(processor != nullptr);

	// Ask for the raster maps input folder
	std::string inputPath;
#ifndef _WIN64
	if (!isWinXPorOlder) {
#endif
		CFolderPickerDialog dlgFolder(NULL, OFN_PATHMUSTEXIST, (CWnd*)this);
		if (dlgFolder.DoModal() == IDOK) inputPath = CT2CA(dlgFolder.GetFolderPath());
		else return;
#ifndef _WIN64
	}
	else if (!BrowseForFolderDialog(inputPath)) return;
#endif
	assert(!inputPath.empty());

	boost::filesystem::path root(inputPath);
	if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen
	for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
		if (boost::filesystem::is_regular_file(*it) && boost::iequals(it->path().extension().string(), ".dem"))
			converter->AddTerrainRasterMapFile(it->path().string());
	}
	if (processor != nullptr && processor->LoadDEMfiles()) StartBusy();
	else MessageBox(_T("Error while starting read raster maps thread."), _T("Error"), MB_ICONERROR);
}

void CAirspaceConverterDlg::OnBnClickedClearInputBt() {
	assert(converter != nullptr);
	converter->UnloadAirspaces();
	assert(converter->GetNumOfAirspaces() == 0);
	if(numWaypointsLoaded == 0) outputFile.clear();
	UpdateOutputFilename();
	LogMessage("Unloaded input airspaces.");
	EndBusy();
}

void CAirspaceConverterDlg::OnBnClickedClearWaypointsBt() {
	assert(converter != nullptr);
	converter->UnloadWaypoints();
	assert(converter->GetNumOfWaypoints() == 0);
	if (numAirspacesLoaded == 0) outputFile.clear();
	LogMessage("Unloaded input waypoints.");
	EndBusy();
}

void CAirspaceConverterDlg::OnBnClickedClearMapsBt() {
	assert(converter != nullptr);
	converter->UnloadRasterMaps();
	assert(converter->GetNumOfTerrainMaps() == 0);
	LogMessage("Unloaded terrain raster maps.");
	EndBusy();
}

void CAirspaceConverterDlg::OnBnClickedFilterBt() {
	assert(converter != nullptr);
	CLimitsDlg dlgLimits;
	dlgLimits.DoModal();
	if (dlgLimits.HasValidLimits()) {
		StartBusy();
		converter->FilterOnLatLonLimits(dlgLimits.GetTopLatLimit(), dlgLimits.GetBottomLatLimit(), dlgLimits.GetLeftLonLimit(), dlgLimits.GetRightLonLimit());
		EndBusy();
	}
}

void CAirspaceConverterDlg::OnBnClickedOutputTypeCombo() {
	UpdateOutputFilename();
	const BOOL isKmzFile(OutputTypeCombo.GetCurSel() == AirspaceConverter::OutputType::KMZ_Format);
	const BOOL isOpenAirFile(OutputTypeCombo.GetCurSel() == AirspaceConverter::OutputType::OpenAir_Format);
	const BOOL isSeeYouFile(OutputTypeCombo.GetCurSel() == AirspaceConverter::OutputType::SeeYou_Format);
	loadInputFileBt.EnableWindow(!isSeeYouFile);
	loadWaypointsBt.EnableWindow(isKmzFile || isSeeYouFile);
	loadDEMfileBt.EnableWindow(isKmzFile);
	LoadAirspacesFolderBt.EnableWindow(!isSeeYouFile);
	loadWaypointsFolderBt.EnableWindow(isKmzFile || isSeeYouFile);
	LoadRasterMapsFolderBt.EnableWindow(isKmzFile);
	editQNHtextField.EnableWindow(isKmzFile ? numAirspacesLoaded == 0 : FALSE);
	editDefualtAltTextField.EnableWindow(isKmzFile);
	pointsCheckBox.EnableWindow(isOpenAirFile);
	secondsCheckBox.EnableWindow(isOpenAirFile);
	ConvertBt.EnableWindow((numAirspacesLoaded > 0 || ((isKmzFile || isSeeYouFile) && numWaypointsLoaded > 0)) && !outputFile.empty() ? TRUE : FALSE);
	UpdateData(FALSE);
}

void CAirspaceConverterDlg::OnBnClickedConvert() {
	if (!UpdateData(TRUE)) return; // Force the user to enter valid default terrain altitude
	assert(!outputFile.empty());
	
	// Prepare and show the open file dialog asking where the user wants to save the converted file
	boost::filesystem::path outputPath(outputFile);
	CFileDialog dlg(FALSE, NULL, CString(outputPath.stem().c_str()) , OFN_HIDEREADONLY, _T("KMZ|*.kmz|OpenAir|*.txt|SeeYou|*.cup|Polish|*.mp|Garmin|*.img||"), (CWnd*)this, 0, TRUE);
	dlg.GetOFN().lpstrTitle = L"Convert to ...";
	dlg.GetOFN().lpstrInitialDir = CString(outputPath.parent_path().c_str());
	dlg.GetOFN().nFilterIndex = OutputTypeCombo.GetCurSel() + 1; // Preselect the same type selected in the combo box
	if (dlg.DoModal() == IDOK) {
		outputFile = CT2CA(dlg.GetPathName());
		outputPath = outputFile;
		AirspaceConverter::OutputType type = AirspaceConverter::DetermineType(outputFile); // Extension really typed in by the user
		if (type != AirspaceConverter::OutputType::Unknown_Format) OutputTypeCombo.SetCurSel(type);
		else { // otherwise force it to the selected extension from the open file dialog
			assert(dlg.GetOFN().nFilterIndex > AirspaceConverter::OutputType::KMZ_Format && dlg.GetOFN().nFilterIndex <= AirspaceConverter::OutputType::Unknown_Format);
			type = (AirspaceConverter::OutputType)(dlg.GetOFN().nFilterIndex - 1);
			AirspaceConverter::PutTypeExtension(type, outputFile);
			OutputTypeCombo.SetCurSel(type);
		}
		OnBnClickedOutputTypeCombo();
	} else return;

	// Continue with the conversion
	if (outputFile.empty()) return;
	AirspaceConverter::OutputType type = (AirspaceConverter::OutputType)OutputTypeCombo.GetCurSel();
	assert(type >= AirspaceConverter::OutputType::KMZ_Format && type < AirspaceConverter::OutputType::Unknown_Format);
	if (boost::filesystem::exists(outputPath)) { // check if file already exists
		CString msg(outputFile.c_str());
		msg += "\nalready exists, overwrite?";
		if (MessageBox(msg, _T("Overwrite?"), MB_YESNO | MB_ICONINFORMATION) == IDYES) std::remove(outputFile.c_str());
		else return;
	}
	assert(converter != nullptr);
	assert(processor != nullptr);
	converter->SetOutputFile(outputFile);
	switch (type) {
	case AirspaceConverter::OutputType::KMZ_Format:
		{
			boost::filesystem::path kmlPath(outputPath.parent_path() / boost::filesystem::path("doc.kml"));
			if (boost::filesystem::exists(kmlPath)) {
				if (MessageBox(_T("In the output folder the file doc.kml already exists.\nIn order to make the KMZ it will be overwritten and deleted. Continue?"), _T("Overwrite?"), MB_YESNO | MB_ICONINFORMATION) == IDYES) std::remove(kmlPath.string().c_str());
				else return;
			}
		}
		converter->SetDefaultTearrainAlt(defaultTerrainAlt);
		if (processor != nullptr && processor->Convert()) StartBusy();
		else MessageBox(_T("Error while starting KML output thread."), _T("Error"), MB_ICONERROR);
		break;
	case AirspaceConverter::OpenAir_Format:
		converter->DoNotCalculateArcsAndCirconferences(pointsCheckBox.GetCheck() == BST_CHECKED);
		converter->WriteCoordinatesAsDDMMSS(secondsCheckBox.GetCheck() == BST_CHECKED);
		if (processor != nullptr && processor->Convert()) StartBusy();
		else MessageBox(_T("Error while starting Polish output thread."), _T("Error"), MB_ICONERROR);
		break;
	case AirspaceConverter::OutputType::SeeYou_Format:
		if (processor != nullptr && processor->Convert()) StartBusy();
		break;
	case AirspaceConverter::OutputType::Garmin_Format:
		{
			boost::filesystem::path polishPath(outputPath);
			polishPath.replace_extension(".mp");
			if (boost::filesystem::exists(polishPath)) {
				CString msg(polishPath.c_str());
				msg += " already exists.\nIn order to make the IMG it will be overwritten and deleted. Continue?";
					if (MessageBox(msg, _T("Overwrite?"), MB_YESNO | MB_ICONINFORMATION) == IDYES) std::remove(polishPath.string().c_str());
				else return;
			}
		} // Fall trough
		/* no break */
	case AirspaceConverter::OutputType::Polish_Format:
		if (processor != nullptr && processor->Convert()) StartBusy();
		else MessageBox(_T("Error while starting Polish output thread."), _T("Error"), MB_ICONERROR);
		break;
	default:
		assert(false);
		break;
	}
}

void CAirspaceConverterDlg::OnBnClickedOpenOutputFile() {
	if (outputFile.empty()) return;
	ShellExecute(0, 0, CString(outputFile.c_str()), 0, 0, SW_SHOW);
}

void CAirspaceConverterDlg::OnBnClickedOpenOutputFolder() {
	if (outputFile.empty()) return;
	ITEMIDLIST* pidl = static_cast<ITEMIDLIST*>(ILCreateFromPathW(CString(outputFile.c_str())));
	if (pidl != nullptr) {
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
}
