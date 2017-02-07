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
#include "afxdialogex.h"
#include "AirspaceConverterApp.h"
#include "AirspaceConverterDlg.h"
#include "Processor.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "AirspaceConverter.h"

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
	CDialog::OnInitDialog();
	this->GetDlgItem(IDC_VERSION)->SetWindowTextW(_T(VERSION));
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
	DDX_Control(pDX, IDC_OUTPUT_FILE_BT, chooseOutputFileBt);
	DDX_Control(pDX, IDC_EDIT_OUTPUT_FILENAME, outputFileEditBox);
	DDX_Control(pDX, IDC_CLEAR_LOG_BT, ClearLogBt);
	DDX_Control(pDX, IDC_LOG, LoggingBox);
	DDX_Control(pDX, IDC_COMBO_OUTPUT_TYPE, OutputTypeCombo);
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
	ON_BN_CLICKED(IDC_OUTPUT_FILE_BT, &CAirspaceConverterDlg::OnBnClickedChooseOutputFileBt)
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
	OutputTypeCombo.InsertString(-1, _T("KMZ format for Google Earth"));
	OutputTypeCombo.InsertString(-1, _T("OpenAir"));
	OutputTypeCombo.InsertString(-1, _T("Polish format for cGPSmapper"));
	OutputTypeCombo.InsertString(-1, _T("IMG file for Garmin devices"));
	OutputTypeCombo.SetCurSel(AirspaceConverter::KMZ);

	// Check if is running on Windows XP (v 5.2) or older. Only on the 32 bit version, on 64 bit we assume that we are using something newer than WinXP
#ifndef _WIN64
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO*)&osvi) && osvi.dwMajorVersion * 10 + osvi.dwMinorVersion <= 52) isWinXPorOlder = true;
	

	// In case of Windows XP disable button that can't be used
	if (isWinXPorOlder) {
		LoadAirspacesFolderBt.EnableWindow(FALSE);
		LoadRasterMapsFolderBt.EnableWindow(FALSE);
		OpenOutputFileBt.EnableWindow(FALSE);
		OpenOutputFolderBt.EnableWindow(FALSE);
	}
#endif

	// Initialize progress bar, necessary to do that that here to make it work properly also on WindowsXP
	progressBar.SetPos(0);
	progressBar.SetMarquee(FALSE, 1);
	progressBar.ModifyStyle(PBS_MARQUEE, 0); // MARQUEE set here, not in the resource file

	// Set the logging function (to write in the logging texbox)
	std::function<void(const std::string&, const bool)> func = std::bind(&CAirspaceConverterDlg::LogMessage, this, std::placeholders::_1, std::placeholders::_2);
	AirspaceConverter::SetLogMessageFuntion(func);

	// Buld the "converter"
	converter = new AirspaceConverter();

	// Buld the "processor"
	processor = new Processor(this->GetSafeHwnd(), converter);	
	if (converter == nullptr || processor == nullptr) MessageBox(_T("Fatal error while initilizing the application."), _T("Error"), MB_ICONERROR);

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

void CAirspaceConverterDlg::StartBusy() {
	busy = true;
	progressBar.ModifyStyle(0, PBS_MARQUEE);
	progressBar.SetMarquee(TRUE, 1);
	loadInputFileBt.EnableWindow(FALSE);
	loadWaypointsBt.EnableWindow(FALSE);
	loadDEMfileBt.EnableWindow(FALSE);
#ifndef _WIN64
	if (!isWinXPorOlder) {
#endif
		LoadAirspacesFolderBt.EnableWindow(FALSE);
		loadWaypointsFolderBt.EnableWindow(FALSE);
		LoadRasterMapsFolderBt.EnableWindow(FALSE);
		OpenOutputFileBt.EnableWindow(FALSE);
		OpenOutputFolderBt.EnableWindow(FALSE);
#ifndef _WIN64
	}
#endif
	unloadAirspacesBt.EnableWindow(FALSE);
	unloadWaypointsBt.EnableWindow(FALSE);
	unloadRasterMapsBt.EnableWindow(FALSE);
	OutputTypeCombo.EnableWindow(FALSE);
	editQNHtextField.EnableWindow(FALSE);
	editDefualtAltTextField.EnableWindow(FALSE);
	chooseOutputFileBt.EnableWindow(FALSE);
	ClearLogBt.EnableWindow(FALSE);
	CloseButton.EnableWindow(FALSE);
}

LRESULT CAirspaceConverterDlg::OnEndJob(WPARAM, LPARAM) {
	conversionDone = converter->IsConversionDone();
	EndBusy();
	return LRESULT();
}

void CAirspaceConverterDlg::UpdateOutputFilename() {
	conversionDone = false;
	if (outputFile.empty()) outputFileEditBox.SetWindowTextW(_T(""));
	else {
		boost::filesystem::path outputPath(outputFile);
		switch ((AirspaceConverter::OutputType)OutputTypeCombo.GetCurSel())
		{
		case AirspaceConverter::KMZ:
			outputPath.replace_extension(".kmz");
			break;
		case AirspaceConverter::OpenAir_Format:
			outputPath.replace_extension(".txt");
			break;
		case AirspaceConverter::Polish:
			outputPath.replace_extension(".mp");
			break;
		case AirspaceConverter::Garmin:
			outputPath.replace_extension(".img");
			break;
		default:
			assert(false);
			outputFile.clear();
			outputFileEditBox.SetWindowTextW(_T(""));
			return;
		}
		outputFile = outputPath.string();
		outputFileEditBox.SetWindowTextW(CString(outputFile.c_str()));
	}
}

void CAirspaceConverterDlg::LogMessage(const std::string& text, const bool isError /*= false*/) {	
	CString message(CA2T((text + '\n').c_str()));
	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = isError ? CFM_BOLD | CFM_COLOR : CFM_COLOR;
	cf.dwEffects = isError ? CFE_BOLD | ~CFE_AUTOCOLOR : ~CFE_AUTOCOLOR;
	cf.crTextColor = isError ? RGB(255, 0, 0) : RGB(0, 0, 0);
	LoggingBox.SetSel(-1, -1); // Set the cursor to the end of the text area and deselect everything.
	LoggingBox.SetSelectionCharFormat(cf);
	LoggingBox.ReplaceSel(message); // Inserts when nothing is selected.
	int linesToScroll = LoggingBox.GetLineCount() - LoggingBox.GetFirstVisibleLine() - 13;
	if (linesToScroll > 0) LoggingBox.LineScroll(linesToScroll);
}

void CAirspaceConverterDlg::OnBnClickedClearLogBt() {
	LoggingBox.SetWindowTextW(_T(""));
}

void CAirspaceConverterDlg::EndBusy() {
	assert(converter != nullptr);
	assert(processor != nullptr);
	if (processor != nullptr) processor->Join();
	if(converter != nullptr) {
		numAirspacesLoaded = converter->GetNumOfAirspaces();
		numWaypointsLoaded = converter->GetNumOfWaypoints();
		numRasterMapLoaded = converter->GetNumOfTerrainMaps();
	} else {
		numAirspacesLoaded = 0;
		numRasterMapLoaded = 0;
	}
	BOOL isKmzFile(OutputTypeCombo.GetCurSel() == AirspaceConverter::KMZ);
	loadInputFileBt.EnableWindow(TRUE);
	loadWaypointsBt.EnableWindow(isKmzFile);
	loadDEMfileBt.EnableWindow(isKmzFile);
#ifndef _WIN64
	if (!isWinXPorOlder) {
#endif
		LoadAirspacesFolderBt.EnableWindow(TRUE);
		loadWaypointsFolderBt.EnableWindow(isKmzFile);
		LoadRasterMapsFolderBt.EnableWindow(isKmzFile);
		OpenOutputFileBt.EnableWindow(conversionDone);
		OpenOutputFolderBt.EnableWindow(conversionDone);
#ifndef _WIN64
	}
#endif
	unloadAirspacesBt.EnableWindow(numAirspacesLoaded > 0 ? TRUE : FALSE);
	unloadWaypointsBt.EnableWindow(numWaypointsLoaded > 0 ? TRUE : FALSE);
	unloadRasterMapsBt.EnableWindow(numRasterMapLoaded > 0 ? TRUE : FALSE);
	ConvertBt.EnableWindow((numAirspacesLoaded > 0 || (isKmzFile && numWaypointsLoaded > 0)) && !outputFile.empty() ? TRUE : FALSE);
	OutputTypeCombo.EnableWindow(TRUE);
	editQNHtextField.EnableWindow(isKmzFile ? numAirspacesLoaded == 0 : FALSE);
	editDefualtAltTextField.EnableWindow(isKmzFile);
	chooseOutputFileBt.EnableWindow(numAirspacesLoaded > 0 || (isKmzFile && numWaypointsLoaded > 0) ? TRUE : TRUE);
	ClearLogBt.EnableWindow(TRUE);
	CloseButton.EnableWindow(TRUE);
	progressBar.SetMarquee(FALSE, 1);
	progressBar.ModifyStyle(PBS_MARQUEE, 0);
	progressBar.SetPos(0);
	UpdateData(FALSE);
	busy = false;
}

void CAirspaceConverterDlg::OnBnClickedInputFile() {
	CFileDialog dlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST, _T("All airspace files|*.txt; *.aip|OpenAIP|*.aip|OpenAir|*.txt||"), (CWnd*)this, 0, TRUE);
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
	CFolderPickerDialog dlgFolder(NULL, OFN_PATHMUSTEXIST, (CWnd*)this);
	if (dlgFolder.DoModal() == IDOK) {
		outputFile.clear();
		conversionDone = false;
		boost::filesystem::path root(dlgFolder.GetFolderPath());
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
}

void CAirspaceConverterDlg::OnBnClickedInputWaypointsFolderBt() {
	assert(converter != nullptr);
	assert(processor != nullptr);
	CFolderPickerDialog dlgFolder(NULL, OFN_PATHMUSTEXIST, (CWnd*)this);
	if (dlgFolder.DoModal() == IDOK) {
		boost::filesystem::path root(dlgFolder.GetFolderPath());
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
}

void CAirspaceConverterDlg::OnBnClickedLoadDemFolderBt() {
	assert(converter != nullptr);
	assert(processor != nullptr);
	CFolderPickerDialog dlgFolder(NULL, OFN_PATHMUSTEXIST, (CWnd*)this);
	if (dlgFolder.DoModal() == IDOK) {
		boost::filesystem::path root(dlgFolder.GetFolderPath());
		if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen
		for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
			if (boost::filesystem::is_regular_file(*it) && boost::iequals(it->path().extension().string(), ".dem"))
				converter->AddTerrainRasterMapFile(it->path().string());
		}
		if (processor != nullptr && processor->LoadDEMfiles()) StartBusy();
		else MessageBox(_T("Error while starting read raster maps thread."), _T("Error"), MB_ICONERROR);
	}
}

void CAirspaceConverterDlg::OnBnClickedClearInputBt() {
	assert(converter != nullptr);
	converter->UnloadAirspaces();
	numAirspacesLoaded = 0;
	assert(numAirspacesLoaded == converter->GetNumOfAirspaces());
	if(numWaypointsLoaded == 0) outputFile.clear();
	UpdateOutputFilename();
	LogMessage("Unloaded input airspaces.");
	EndBusy();
}

void CAirspaceConverterDlg::OnBnClickedClearWaypointsBt() {
	assert(converter != nullptr);
	converter->UnloadWaypoints();
	numWaypointsLoaded = 0;
	assert(numWaypointsLoaded == converter->GetNumOfWaypoints());
	if (numAirspacesLoaded == 0) outputFile.clear();
	LogMessage("Unloaded input waypoints.");
	EndBusy();
}

void CAirspaceConverterDlg::OnBnClickedClearMapsBt() {
	assert(converter != nullptr);
	converter->UnloadRasterMaps();
	numRasterMapLoaded = 0;
	assert(numRasterMapLoaded == converter->GetNumOfTerrainMaps());
	LogMessage("Unloaded raster maps.");
	EndBusy();
}

void CAirspaceConverterDlg::OnBnClickedChooseOutputFileBt() {
	assert(!outputFile.empty());
	boost::filesystem::path outputPath(outputFile);
	std::string ext(outputPath.extension().string()); // This should be the same as type from the dialog combo box, preselect this type in the open file dlg
	assert(ext.length() > 1);
	assert(ext.at(0) == '.');
	CFileDialog dlg(FALSE, NULL, CString(std::string("*" + ext).c_str()), OFN_HIDEREADONLY, _T("KMZ|*.kmz|OpenAir|*.txt|Polish|*.mp|Garmin|*.img||"), (CWnd*)this, 0, TRUE);
	if (dlg.DoModal() == IDOK) {
		outputFile = CT2CA(dlg.GetPathName());
		outputPath = outputFile;
		ext = outputPath.extension().string(); // Extension really typed in by the user
		if (boost::iequals(ext, ".kmz")) OutputTypeCombo.SetCurSel(AirspaceConverter::KMZ);
		else if (boost::iequals(ext, ".txt")) OutputTypeCombo.SetCurSel(AirspaceConverter::OpenAir_Format);
		else if (boost::iequals(ext, ".mp")) OutputTypeCombo.SetCurSel(AirspaceConverter::Polish);
		else if (boost::iequals(ext, ".img")) OutputTypeCombo.SetCurSel(AirspaceConverter::Garmin);
		else { // otherwise force it to the selected extension from the open file dialog
			assert(dlg.GetOFN().nFilterIndex > AirspaceConverter::KMZ && dlg.GetOFN().nFilterIndex <= AirspaceConverter::NumOfOutputTypes);
			AirspaceConverter::OutputType type = (AirspaceConverter::OutputType)(dlg.GetOFN().nFilterIndex - 1);
			switch (type) {
			case AirspaceConverter::KMZ:
				outputFile = outputPath.replace_extension(".kmz").string();
				break;
			case AirspaceConverter::OpenAir_Format:
				outputFile = outputPath.replace_extension(".txt").string();
				break;
			case AirspaceConverter::Polish:
				outputFile = outputPath.replace_extension(".mp").string();
				break;
			case AirspaceConverter::Garmin:
				outputFile = outputPath.replace_extension(".img").string();
				break;
			default:
				assert(false);
				return;
			}
			OutputTypeCombo.SetCurSel(type);
		}
		OnBnClickedOutputTypeCombo();
	}
}

void CAirspaceConverterDlg::OnBnClickedOutputTypeCombo() {
	UpdateOutputFilename();
	BOOL isKmzFile(OutputTypeCombo.GetCurSel() == AirspaceConverter::KMZ);
	loadWaypointsBt.EnableWindow(isKmzFile);
	loadDEMfileBt.EnableWindow(isKmzFile);
#ifndef _WIN64
	if (!isWinXPorOlder) {
#endif
		loadWaypointsFolderBt.EnableWindow(isKmzFile);
		LoadRasterMapsFolderBt.EnableWindow(isKmzFile);
#ifndef _WIN64
	}
#endif
	editQNHtextField.EnableWindow(isKmzFile ? numAirspacesLoaded == 0 : FALSE);
	editDefualtAltTextField.EnableWindow(isKmzFile);
	ConvertBt.EnableWindow((numAirspacesLoaded > 0 || (isKmzFile && numWaypointsLoaded > 0)) && !outputFile.empty() ? TRUE : FALSE);
	UpdateData(FALSE);
}

void CAirspaceConverterDlg::OnBnClickedConvert() {
	if (outputFile.empty()) return;
	AirspaceConverter::OutputType type = (AirspaceConverter::OutputType)OutputTypeCombo.GetCurSel();
	assert(type >= AirspaceConverter::KMZ && type < AirspaceConverter::NumOfOutputTypes);
	boost::filesystem::path outputPath(outputFile);
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
	case AirspaceConverter::KMZ:
		{
			boost::filesystem::path kmlPath(outputPath);
			kmlPath.replace_extension(".kml");
			if (boost::filesystem::exists(kmlPath)) {
				CString msg(kmlPath.c_str());
				msg += " already exists.\nIn order to make the KMZ it needs to be overwritten. Continue?";
				if (MessageBox(msg, _T("Overwrite?"), MB_YESNO | MB_ICONINFORMATION) == IDYES) std::remove(kmlPath.string().c_str());
				else return;
			}
		}
		converter->SetDefaultTearrainAlt(defaultTerrainAlt);
		if (processor != nullptr && processor->Convert()) StartBusy();
		else MessageBox(_T("Error while starting KML output thread."), _T("Error"), MB_ICONERROR);
		break;
	case AirspaceConverter::OpenAir_Format:
		converter->SetOutputFile(outputFile);
		if (processor != nullptr && processor->Convert()) StartBusy();
		else MessageBox(_T("Error while starting Polish output thread."), _T("Error"), MB_ICONERROR);
		break;
	case AirspaceConverter::Garmin:
		{
			boost::filesystem::path polishPath(outputPath);
			polishPath.replace_extension(".mp");
			if (boost::filesystem::exists(polishPath)) {
				CString msg(polishPath.c_str());
				msg += " already exists.\nIn order to make the IMG it needs to be overwritten. Continue?";
					if (MessageBox(msg, _T("Overwrite?"), MB_YESNO | MB_ICONINFORMATION) == IDYES) std::remove(polishPath.string().c_str());
				else return;
			}
		} // Fall trough
	case AirspaceConverter::Polish:
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
