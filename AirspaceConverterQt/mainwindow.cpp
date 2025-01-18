//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "mainwindow.h"
#include "ui_mainwindow.h"

#if QT_VERSION >= 0x050000
    #include <QtConcurrent/QtConcurrent>
    #include <QtWidgets>
#else
    #include <QtConcurrentRun> // Use this instead of the previous to compile with older Qt versions
    #include <QUrl>
    #include <QtGui>
#endif

#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QDesktopServices>
#include <QFuture>
#include <QFutureWatcher>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <cassert>
#include "AirspaceConverter.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    converter(new AirspaceConverter()),
    busy(false),
    suggestedInputDir(QDir::homePath()),
    outputFileInsertedViaDialog(false) {
    assert(converter != nullptr);
    assert(ui != nullptr);

    // Set up UI and signals
    ui->setupUi(this);
    connect(this, SIGNAL(messagePosted(QString)), this, SLOT(logMessage(QString)));
    connect(this, SIGNAL(warningPosted(QString)), this, SLOT(logWarning(QString)));
    connect(this, SIGNAL(errorPosted(QString)), this, SLOT(logError(QString)));
    connect(&watcher, SIGNAL(finished()), this, SLOT(endBusy()));
    connect(&filter, SIGNAL(validLimitsSet(double, double, double, double)), this, SLOT(applyFilter(double, double, double, double)));

    // Set the logging functions (to write in the logging texbox)
    AirspaceConverter::SetLogMessageFunction(std::function<void(const std::string&)>(std::bind(&MainWindow::postMessage, this, std::placeholders::_1)));
    AirspaceConverter::SetLogWarningFunction(std::function<void(const std::string&)>(std::bind(&MainWindow::postWarning, this, std::placeholders::_1)));
    AirspaceConverter::SetLogErrorFunction(std::function<void(const std::string&)>(std::bind(&MainWindow::postError, this, std::placeholders::_1)));

    // Enable the option to make a Garmin IMG file only if cGPSmapper is available
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->outputFormatComboBox->model());
    assert(model != nullptr);
    QStandardItem* item = model->item((int)AirspaceConverter::Garmin_Format);
    assert(item != nullptr);
    item->setFlags(AirspaceConverter::Is_cGPSmapperAvailable() ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled);
}

MainWindow::~MainWindow() {
    if (ui != nullptr) delete ui;
    if (converter != nullptr) delete converter;
}

// These are the "local" MainWindow members functons to append a new message on the log
void MainWindow::logMessage(const QString& message) {
    ui->loggingTextBox->setTextColor(Qt::black);
    appendAndScrollLog(message);
}

void MainWindow::logWarning(const QString& message) {
    ui->loggingTextBox->setTextColor(Qt::darkYellow);
    appendAndScrollLog("Warning: " + message);
}

void MainWindow::logError(const QString& message) {
    ui->loggingTextBox->setTextColor(Qt::red);
    appendAndScrollLog("ERROR: " + message);
}

void MainWindow::appendAndScrollLog(const QString& text) {
    ui->loggingTextBox->append(text);
    ui->loggingTextBox->verticalScrollBar()->setValue(ui->loggingTextBox->verticalScrollBar()->maximum());
}

// These are the functions that will be called by libAirspaceConverter from another thread to append a new message on the log
void MainWindow::postMessage(const std::string& message) {
    emit messagePosted(QString::fromUtf8(message.c_str()));  // Emit the signal that a new message has to be posted on the log (multiple signals will be queued)
}

void MainWindow::postWarning(const std::string& message) {
    emit warningPosted(QString::fromUtf8(message.c_str()));
}

void MainWindow::postError(const std::string& message) {
    emit errorPosted(QString::fromUtf8(message.c_str()));
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if(busy) {
        QMessageBox::StandardButton resBtn = QMessageBox::warning(this, "AirspaceConverter", tr("AirspaceConverter is still working!\nAre you really sure to exit?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
        if (resBtn != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::startBusy() {
    busy = true;

    // Marquee progrees bar
    ui->progressBar->setMaximum(0);

    // Disable all
    ui->outputFormatComboBox->setEnabled(false);
    ui->loadAirspaceFileButton->setEnabled(false);
    ui->loadAirspaceFolderButton->setEnabled(false);
    ui->unloadAirspacesButton->setEnabled(false);
    ui->loadWaypointFileButton->setEnabled(false);
    ui->loadWaypointsFolderButton->setEnabled(false);
    ui->unloadWaypointsButton->setEnabled(false);
    ui->loadRasterMapFileButton->setEnabled(false);
    ui->loadRasterMapFolderButton->setEnabled(false);
    ui->unloadTerrainMapsButton->setEnabled(false);
    ui->filterButton->setEnabled(false);
    ui->defaultAltSpinBox->setEnabled(false);
    ui->QNHspinBox->setEnabled(false);
    ui->onlyPointsCheckBox->setEnabled(false);
    ui->openAirCoordinateTypeComboBox->setEnabled(false);
    ui->convertButton->setEnabled(false);
    ui->openOutputFileButton->setEnabled(false);
    ui->openOutputFolderButton->setEnabled(false);
    ui->clearLogButton->setEnabled(false);
    ui->closeButton->setEnabled(false);

    // Start the timer
    startTime = std::chrono::high_resolution_clock::now();
}

void MainWindow::refreshUI() {
    // Find out pre-conditions
    const bool airspaceOutput(ui->outputFormatComboBox->currentIndex() != AirspaceConverter::SeeYou_Format && ui->outputFormatComboBox->currentIndex() != AirspaceConverter::CSV_Format);
    const bool waypointsOutput(ui->outputFormatComboBox->currentIndex() == AirspaceConverter::KMZ_Format || ui->outputFormatComboBox->currentIndex() == AirspaceConverter::SeeYou_Format || ui->outputFormatComboBox->currentIndex() == AirspaceConverter::CSV_Format);
    const bool isKMZ(ui->outputFormatComboBox->currentIndex() == AirspaceConverter::KMZ_Format);
    const bool isOpenAir(ui->outputFormatComboBox->currentIndex() == AirspaceConverter::OpenAir_Format);

    // Re-enable all specifically
    ui->outputFormatComboBox->setEnabled(true);
    ui->loadAirspaceFileButton->setEnabled(airspaceOutput);
    ui->loadAirspaceFolderButton->setEnabled(airspaceOutput);
    ui->unloadAirspacesButton->setEnabled(converter->GetNumOfAirspaces()>0);
    ui->loadWaypointFileButton->setEnabled(waypointsOutput);
    ui->loadWaypointsFolderButton->setEnabled(waypointsOutput);
    ui->unloadWaypointsButton->setEnabled(converter->GetNumOfWaypoints()>0);
    ui->loadRasterMapFileButton->setEnabled(waypointsOutput);
    ui->loadRasterMapFolderButton->setEnabled(waypointsOutput);
    ui->unloadTerrainMapsButton->setEnabled(converter->GetNumOfTerrainMaps()>0);
    ui->filterButton->setEnabled(converter->GetNumOfAirspaces()>0 || converter->GetNumOfWaypoints()>0);
    ui->defaultAltSpinBox->setEnabled(isKMZ);
    ui->defaultTerrainLabel->setEnabled(isKMZ);
    ui->meterLabel->setEnabled(isKMZ);
    ui->qnhLabel->setEnabled(airspaceOutput);
    ui->QNHspinBox->setEnabled(airspaceOutput && converter->GetNumOfAirspaces()==0);
    ui->hPaLabel->setEnabled(airspaceOutput);
    ui->onlyPointsCheckBox->setEnabled(isOpenAir);
    ui->openAirCoordinateTypeComboBox->setEnabled(isOpenAir);
    ui->openAirCoordonateFormatLabel->setEnabled(isOpenAir);
    ui->convertButton->setEnabled(!converter->GetOutputFile().empty() && ((airspaceOutput && converter->GetNumOfAirspaces()>0) || (waypointsOutput && converter->GetNumOfWaypoints()>0)));
    ui->openOutputFileButton->setEnabled(converter->IsConversionDone());
    ui->openOutputFolderButton->setEnabled(converter->IsConversionDone());
    ui->clearLogButton->setEnabled(true);
    ui->closeButton->setEnabled(true);
}

void MainWindow::endBusy() {
    if (busy) { // Stop the timer
        const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
        logMessage(QString::fromStdString(std::string(boost::str(boost::format("Execution time: %1f sec.") %elapsedTimeSec))));
    }

    // Set the numer of airspaces loaded in its spinBox
    ui->numAirspacesLoadedSpinBox->setValue(converter->GetNumOfAirspaces());

    // Set the number of waypoints loaded in its spinBox
    ui->numWaypointsLoadedSpinBox->setValue(converter->GetNumOfWaypoints());

    // Set the number of terrain raster maps loaded in its spinBox
    ui->numTerrainMapsLoadedSpinBox->setValue(converter->GetNumOfTerrainMaps());

    // Re-enable buttons and UI
    refreshUI();

    if (busy) {
        ui->progressBar->setMaximum(100); // This will disable marquee progrees bar
        busy = false;
    }
}

void MainWindow::on_aboutButton_clicked() {
    about.show();
}

void MainWindow::on_outputFormatComboBox_currentIndexChanged(int index) {
    converter->SetOutputType((AirspaceConverter::OutputType)index);
    refreshUI();
}

void MainWindow::on_loadAirspaceFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Airspace files"), suggestedInputDir, tr("All airspace files(*.txt *.TXT *.aip *.AIP *.kml *.KML *.kmz *.KMZ);;OpenAir(*.txt *.TXT);;openAIP(*.aip *.AIP);;Google Earth(*.kml *.KML *.kmz *.KMZ);;") );
    if(filenames.empty()) return;

    // Start to work
    startBusy();

    // Remember the directory
    suggestedInputDir = QString::fromStdString(boost::filesystem::path(filenames.front().toStdString()).parent_path().string());

    // Set QNH
    converter->SetQNH(ui->QNHspinBox->value());

    // Load all the files
    for(const auto& file : filenames) converter->AddAirspaceFile(file.toStdString());
    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadAirspaces, (AirspaceConverter::OutputType)ui->outputFormatComboBox->currentIndex()));
}

void MainWindow::on_loadAirspaceFolderButton_clicked() {
    const QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Open airspace directory"), suggestedInputDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (selectedDir.isEmpty()) return;

    // Start to work
    startBusy();

    // Remember the directory
    suggestedInputDir = selectedDir;

    // Set QNH
    converter->SetQNH(ui->QNHspinBox->value());

    // Load all the files in the folder
    for (boost::filesystem::directory_iterator it(boost::filesystem::path(selectedDir.toStdString())), endit; it != endit; ++it) {
        if (boost::filesystem::is_regular_file(*it)) {
            const std::string ext = it->path().extension().string();
            if (boost::iequals(ext, ".txt") || boost::iequals(ext, ".aip") || boost::iequals(ext, ".kmz") || boost::iequals(ext, ".kml"))
                converter->AddAirspaceFile(it->path().string());
        }
    }
    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadAirspaces, (AirspaceConverter::OutputType)ui->outputFormatComboBox->currentIndex()));
}

void MainWindow::on_unloadAirspacesButton_clicked() {
    converter->UnloadAirspaces();
    logMessage("Unloaded input airspaces.");
    endBusy();
}

void MainWindow::on_loadWaypointFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Waypoints files"), suggestedInputDir, tr("All waypoints files(*.cup *.CUP *.aip *.AIP *.csv *.CSV);;SeeYou files(*.cup *.CUP);;openAIP(*.aip *.AIP);;LittleNavMap(*.csv *.CSV);;") );

    if(filenames.empty()) return;

    // Start to work...
    startBusy();

    // Remember the directory
    suggestedInputDir = QString::fromStdString(boost::filesystem::path(filenames.front().toStdString()).parent_path().string());

    // Load all the files
    for(const auto& file : filenames) converter->AddWaypointFile(file.toStdString());
    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadWaypoints));
}

void MainWindow::on_loadWaypointsFolderButton_clicked() {
    const QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Open waypoints directory"), suggestedInputDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (selectedDir.isEmpty()) return;

    // Start to work...
    startBusy();

    // Remember the directory
    suggestedInputDir = selectedDir;

    // Load all the files in the folder (.cup only)
    for (boost::filesystem::directory_iterator it(boost::filesystem::path(selectedDir.toStdString())), endit; it != endit; ++it) {
        if (boost::filesystem::is_regular_file(*it)) {
            const std::string ext = it->path().extension().string();
            if (boost::iequals(ext, ".cup") || boost::iequals(ext, ".aip") || boost::iequals(ext, ".csv"))
                converter->AddWaypointFile(it->path().string());
        }
    }
    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadWaypoints));
}

void MainWindow::on_unloadWaypointsButton_clicked() {
    converter->UnloadWaypoints();
    logMessage("Unloaded input waypoints.");
    endBusy();
}

void MainWindow::on_loadRasterMapFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Terrain raster map files"), suggestedInputDir, tr("DEM files(*.dem *.DEM);;"));
    if(filenames.empty()) return;

    // Start to work...
    startBusy();

    // Remember the directory
    suggestedInputDir = QString::fromStdString(boost::filesystem::path(filenames.front().toStdString()).parent_path().string());

    // Load terrain maps
    for(const auto& mapFile : filenames) converter->AddTerrainRasterMapFile(mapFile.toStdString());
    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadTerrainRasterMaps));
}

void MainWindow::on_loadRasterMapFolderButton_clicked() {
    const QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Open raster terrain maps directory"), suggestedInputDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (selectedDir.isEmpty()) return;

    // Start to work...
    startBusy();

    // Remember the directory
    suggestedInputDir = selectedDir;

    // Load all the .dem files in the folder
    for (boost::filesystem::directory_iterator it(boost::filesystem::path(selectedDir.toStdString())), endit; it != endit; ++it) {
        if (!boost::filesystem::is_regular_file(*it)) continue;
        if(boost::iequals(it->path().extension().string(), ".dem")) converter->AddTerrainRasterMapFile(it->path().string());
    }
    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadTerrainRasterMaps));
}

void MainWindow::on_unloadTerrainMapsButton_clicked() {
    converter->UnloadRasterMaps();
    logMessage("Unloaded input terrain raster maps.");
    endBusy();
}

void MainWindow::on_convertButton_clicked() {
    QString selectedFilter; // this string will conatin the selected type by the user in the dialog

    // The desired format is initially dictated by the combo box: the user will have it already preselected in the file dialog
    switch(ui->outputFormatComboBox->currentIndex()) {
        case AirspaceConverter::OutputType::KMZ_Format:     selectedFilter = tr("Google Earth(*.kmz)"); break;
        case AirspaceConverter::OutputType::OpenAir_Format: selectedFilter = tr("OpenAir(*.txt)"); break;
        case AirspaceConverter::OutputType::SeeYou_Format:  selectedFilter = tr("SeeYou(*.cup)"); break;
        case AirspaceConverter::OutputType::CSV_Format:     selectedFilter = tr("LittleNavMap(*.csv)"); break;
        case AirspaceConverter::OutputType::Polish_Format:  selectedFilter = tr("Polish(*.mp)"); break;
        case AirspaceConverter::OutputType::Garmin_Format:  selectedFilter = tr("Garmin img(*.img)"); break;
        default: assert(false);
    }

    // Prepare dialog to ask for output file, will be without extension if not manually typed by the user
    std::string desiredOutputFile = QFileDialog::getSaveFileName(this, tr("Convert to..."),
                                                                 QString::fromStdString(boost::filesystem::path(converter->GetOutputFile()).replace_extension("").string()),
                                                                 AirspaceConverter::Is_cGPSmapperAvailable() ?
                                                                     tr("Google Earth(*.kmz);;OpenAir(*.txt);;SeeYou(*.cup);;LittleNavMap(*.csv);;Polish(*.mp);;Garmin img(*.img)") :
                                                                     tr("Google Earth(*.kmz);;OpenAir(*.txt);;SeeYou(*.cup);;LittleNavMap(*.csv);;Polish(*.mp)"),
                                                                 &selectedFilter).toStdString();

    // If no file selected or entered: do nothing
    if(desiredOutputFile.empty()) return;

    // Get the type from the extension of selected or entered file
    AirspaceConverter::OutputType desiredFormat = AirspaceConverter::DetermineType(desiredOutputFile);

    // Verify if it is an acceptable extension (in Linux extension it is not added by the dialog if the user doesn't type it, or type it wrong)
    if(desiredFormat == AirspaceConverter::OutputType::Unknown_Format) {

        // In this case, may be, the user typed just a name but selecting the extension in the save as combo box file type
        desiredFormat = AirspaceConverter::KMZ_Format; // KMZ default
        if (selectedFilter != "Google Earth(*.kmz)") {
            if (selectedFilter == "OpenAir(*.txt)") desiredFormat = AirspaceConverter::OpenAir_Format;
            else if (selectedFilter == "SeeYou(*.cup)") desiredFormat = AirspaceConverter::SeeYou_Format;
            else if (selectedFilter == "LittleNavMap(*.csv)") desiredFormat = AirspaceConverter::CSV_Format;
            else if (selectedFilter == "Polish(*.mp)") desiredFormat = AirspaceConverter::Polish_Format;
            else if (selectedFilter == "Garmin img(*.img)") desiredFormat = AirspaceConverter::Garmin_Format;
            else assert(false);
        }

        // ... and use that extension
        AirspaceConverter::PutTypeExtension(desiredFormat, desiredOutputFile);
    }

    // Set the output file
    converter->SetOutputFile(desiredOutputFile);
    assert(converter->GetOutputType() == desiredFormat);

    // Reselect the desired format also in the combo box
    ui->outputFormatComboBox->setCurrentIndex((int)desiredFormat);

    // Remember that the file was inserted via save as dialog in order to don't ask twice to overwrite it
    outputFileInsertedViaDialog = true;

    // Proceed with conversion
    const bool conversionPossible (!converter->GetOutputFile().empty() &&
        ((ui->outputFormatComboBox->currentIndex() != AirspaceConverter::SeeYou_Format && ui->outputFormatComboBox->currentIndex() != AirspaceConverter::CSV_Format && converter->GetNumOfAirspaces()>0) ||
        ((ui->outputFormatComboBox->currentIndex() == AirspaceConverter::KMZ_Format || ui->outputFormatComboBox->currentIndex() == AirspaceConverter::SeeYou_Format || ui->outputFormatComboBox->currentIndex() == AirspaceConverter::CSV_Format) && converter->GetNumOfWaypoints()>0)));
    assert(conversionPossible);
    if(!conversionPossible) return;

    // Ask confirmation to overwrite the output file
    boost::filesystem::path path(converter->GetOutputFile());
    if(!outputFileInsertedViaDialog && boost::filesystem::exists(path) &&
            QMessageBox::warning(this, "Overwrite?", tr("Selected output file already exists. Overwrite?"), QMessageBox::Yes | QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) return;

    // Ask confirmation to overwrite any other file that will be created during the conversion process
    switch(converter->GetOutputType()) {
        case AirspaceConverter::OutputType::KMZ_Format:
            if(boost::filesystem::exists(boost::filesystem::path(path.parent_path() / boost::filesystem::path("doc.kml"))) && QMessageBox::warning(this, "Overwrite?", tr("The already existing doc.kml file will be deleted, continue?"), QMessageBox::Yes | QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) return;
            break;
        case AirspaceConverter::OutputType::Garmin_Format:
            if(boost::filesystem::exists(path.replace_extension(".mp")) && QMessageBox::warning(this, "Overwrite?", tr("The already existing .MP file will be deleted, continue?"), QMessageBox::Yes | QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) return;
            break;
        default:
            break;
    }

    // Start work...
    startBusy();

    // Set default terrain altitude
    AirspaceConverter::SetDefaultTerrainAlt(ui->defaultAltSpinBox->value());

    // Set OpenAir settings
    converter->DoNotCalculateArcsAndCirconferences(ui->onlyPointsCheckBox->isChecked());
    switch(ui->openAirCoordinateTypeComboBox->currentIndex()) {
        case 0: //OpenAir::CoordinateType::DEG_DECIMAL_MIN
            converter->SetOpenAirCoodinatesInDecimalMinutes();
            break;
        case 1: //OpenAir::CoordinateType::DEG_MIN_SEC
            converter->SetOpenAirCoodinatesInSeconds();
            break;
        default:
            assert(false);
        case 2: //OpenAir::CoordinateType::AUTO
            converter->SetOpenAirCoodinatesAutomatic();
    }

    // Let the libAirspaceConverter to do the work in a separate thread...
    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::Convert));
}

void MainWindow::on_openOutputFileButton_clicked() {    
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(converter->GetOutputFile())));
}

void MainWindow::on_openOutputFolderButton_clicked() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(boost::filesystem::path(converter->GetOutputFile()).parent_path().string())));
}

void MainWindow::on_filterButton_clicked() {
    filter.show();
}

void MainWindow::applyFilter(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon) {
    // Start to work...
    startBusy();

    // Apply filter
    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::FilterOnLatLonLimits, topLat, bottomLat, leftLon, rightLon));
}
