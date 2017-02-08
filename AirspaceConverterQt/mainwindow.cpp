#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QDesktopServices>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <cassert>
#include "AirspaceConverter.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    converter(new AirspaceConverter),
    busy(false) {
    // Set the logging function (to write in the logging texbox)
    std::function<void(const std::string&, const bool)> func = std::bind(&MainWindow::logMessage, this, std::placeholders::_1, std::placeholders::_2);
    AirspaceConverter::SetLogMessageFunction(func);
    assert(converter != nullptr);
    assert(ui != nullptr);
    ui->setupUi(this);
    connect(&watcher, SIGNAL(finished()), this, SLOT(endBusy()));
}

MainWindow::~MainWindow() {
    if (ui != nullptr) delete ui;
    if (converter != nullptr) delete converter;
}

void MainWindow::logMessage(const std::string& message, const bool isError) {
    log(QString::fromStdString(message), isError);
}

void MainWindow::log(const QString& message, const bool isError) {
    // TODO: implement a message queue!!!
    ui->loggingTextBox->setTextColor(isError ? "red" : "black");
    ui->loggingTextBox->append(message);
    ui->loggingTextBox->verticalScrollBar()->setValue(ui->loggingTextBox->verticalScrollBar()->maximum());
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
    ui->defaultAltSpinBox->setEnabled(false);
    ui->QNHspinBox->setEnabled(false);
    ui->chooseOutputFileButton->setEnabled(false);
    ui->convertButton->setEnabled(false);
    ui->openOutputFileButton->setEnabled(false);
    ui->openOutputFolderButton->setEnabled(false);
    ui->clearLogButton->setEnabled(false);
    ui->closeButton->setEnabled(false);

    // Start the timer
    startTime = std::chrono::high_resolution_clock::now();
}

void MainWindow::endBusy() {
    // Stop the timer
    if (busy) {
        const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
        logMessage(std::string(boost::str(boost::format("Execution time: %1f sec.") %elapsedTimeSec)));
    }
    
    // All operartions to do after loading
    if(!converter->IsConversionDone()) {

        // Set the numer of airspaces loaded in its spinBox
        ui->numAirspacesLoadedSpinBox->setValue(converter->GetNumOfAirspaces());

        // Eventually update the output file
        if(ui->outputFileTextEdit->toPlainText().size() == 0) updateOutputFileExtension(ui->outputFormatComboBox->currentIndex());

        // Set the numer of waypoints loaded in its spinBox
        ui->numWaypointsLoadedSpinBox->setValue(converter->GetNumOfWaypoints());
    
        // Set the numer of terrain raster map loaded in its spinBox
        ui->numTerrainMapsLoadedSpinBox->setValue(converter->GetNumOfTerrainMaps());
    }

    // Re-enable all specifically
    ui->outputFormatComboBox->setEnabled(true);
    ui->loadAirspaceFileButton->setEnabled(true);
    ui->loadAirspaceFolderButton->setEnabled(true);
    ui->unloadAirspacesButton->setEnabled(converter->GetNumOfAirspaces()>0);
    ui->loadWaypointFileButton->setEnabled(true);
    ui->loadWaypointsFolderButton->setEnabled(true);
    ui->unloadWaypointsButton->setEnabled(converter->GetNumOfWaypoints()>0);
    ui->loadRasterMapFileButton->setEnabled(true);
    ui->loadRasterMapFolderButton->setEnabled(true);
    ui->unloadTerrainMapsButton->setEnabled(converter->GetNumOfTerrainMaps()>0);
    ui->defaultAltSpinBox->setEnabled(true);
    ui->QNHspinBox->setEnabled(true);
    ui->chooseOutputFileButton->setEnabled(true);
    ui->convertButton->setEnabled(converter->GetNumOfAirspaces()>0 && !converter->GetOutputFile().empty());
    ui->openOutputFileButton->setEnabled(converter->IsConversionDone());
    ui->openOutputFolderButton->setEnabled(converter->IsConversionDone());
    ui->clearLogButton->setEnabled(true);
    ui->closeButton->setEnabled(true);

    if (busy) {
        // Disable marquee progrees bar
        ui->progressBar->setMaximum(100);

        busy = false;
    }
}

void MainWindow::on_aboutButton_clicked() {
    about.show();
}

void MainWindow::on_outputFormatComboBox_currentIndexChanged(int index) {
    updateOutputFileExtension(index);
    endBusy();
}

void MainWindow::updateOutputFileExtension(const int newExtIdx) {
    std::string outputFile(converter->GetOutputFile());
    if(outputFile.empty()) return;
    boost::filesystem::path path(outputFile);
    switch(newExtIdx) {
        case AirspaceConverter::KMZ:
            path.replace_extension(".kmz");
            break;
        case AirspaceConverter::OpenAir_Format:
            path.replace_extension(".txt");
            break;
        case AirspaceConverter::Polish:
            path.replace_extension(".mp");
            break;
        case AirspaceConverter::Garmin:
            path.replace_extension(".img");
            break;
        default:
            assert(false);
    }
    outputFile = path.string();
    ui->outputFileTextEdit->setPlainText(QString::fromStdString(outputFile));
    converter->SetOutputFile(outputFile);
}

void MainWindow::on_loadAirspaceFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Airspace files"), QDir::currentPath(), tr("All airspace files (*.txt *.aip);;OpenAir (*.txt);;OpenAIP (*.aip);;") );
    if(filenames.empty()) return;

    startBusy();

    // Set QNH
    converter->SetQNH(ui->QNHspinBox->value());

    // Load all the files
    for(const auto& file : filenames) converter->AddAirspaceFile(file.toStdString());

    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadAirspaces));
}

void MainWindow::on_loadAirspaceFolderButton_clicked() {
    boost::filesystem::path root(QFileDialog::getExistingDirectory(this, tr("Open airspace directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks).toStdString());
    if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen

    startBusy();

    // Set QNH
    converter->SetQNH(ui->QNHspinBox->value());

    for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
        if (!boost::filesystem::is_regular_file(*it)) continue;
        converter->AddAirspaceFile(it->path().string());
    }

    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadAirspaces));
}

void MainWindow::on_unloadAirspacesButton_clicked() {
    converter->UnloadAirspaces();
    ui->numAirspacesLoadedSpinBox->setValue(0);
    endBusy();
}

void MainWindow::on_loadWaypointFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Waypoints files"), QDir::currentPath(), tr("CUP files(*.cup);;") );
    if(filenames.empty()) return;

    startBusy();

    // Load all the files
    for(const auto& file : filenames) converter->AddWaypointFile(file.toStdString());

    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadWaypoints));
}

void MainWindow::on_loadWaypointsFolderButton_clicked() {
    boost::filesystem::path root(QFileDialog::getExistingDirectory(this, tr("Open waypoints directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks).toStdString());
    if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen

    startBusy();

    for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
        if (!boost::filesystem::is_regular_file(*it)) continue;
        if(boost::iequals(it->path().extension().string(), ".cup")) converter->AddWaypointFile(it->path().string());
    }

    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadWaypoints));
}

void MainWindow::on_unloadWaypointsButton_clicked() {
    converter->UnloadWaypoints();
    ui->numWaypointsLoadedSpinBox->setValue(0);
}

void MainWindow::on_loadRasterMapFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Terrain raster map files"), QDir::currentPath(), tr("DEM files (*.dem);;"));
    if(filenames.empty()) return;

    startBusy();

    // Load terrain maps
    for(const auto& mapFile : filenames) converter->AddTerrainRasterMapFile(mapFile.toStdString());

    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadTerrainRasterMaps));
}

void MainWindow::on_loadRasterMapFolderButton_clicked() {
    boost::filesystem::path root(QFileDialog::getExistingDirectory(this, tr("Open raster terrain maps directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks).toStdString());
    if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen

    startBusy();

    for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
        if (!boost::filesystem::is_regular_file(*it)) continue;
        if(boost::iequals(it->path().extension().string(), ".dem")) converter->AddTerrainRasterMapFile(it->path().string());
    }

    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::LoadTerrainRasterMaps));
}

void MainWindow::on_unloadTerrainMapsButton_clicked() {
    converter->UnloadRasterMaps();
    ui->numTerrainMapsLoadedSpinBox->setValue(0);
}

void MainWindow::on_convertButton_clicked() {
    assert(converter->GetNumOfAirspaces() > 0);
    assert(!converter->GetOutputFile().empty());
    if(converter->GetNumOfAirspaces() ==0 || converter->GetOutputFile().empty()) return;

    startBusy();

    // Set default terrain altitude
    converter->SetDefaultTearrainAlt(ui->defaultAltSpinBox->value());

    watcher.setFuture(QtConcurrent::run(converter, &AirspaceConverter::Convert));
}

void MainWindow::on_openOutputFileButton_clicked() {    
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(converter->GetOutputFile())));
}

void MainWindow::on_openOutputFolderButton_clicked() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(boost::filesystem::path(converter->GetOutputFile()).parent_path().string())));
}

void MainWindow::on_chooseOutputFileButton_clicked() {
    // Prepare dialog to ask for output file, will be without extension if not manually typed by the user
    QString selectedFilter; // this will conatin the selected type by the user in the dialog
    converter->SetOutputFile(QFileDialog::getSaveFileName(this, tr("Output file"), QDir::currentPath(), tr("Google Earth (*.kmz);;OpenAir (*.txt);;Polish (*.mp);;Garmin map (*.img)"), &selectedFilter).toStdString());

    // Determine which format is now selected
    int desiredFormatIndex = AirspaceConverter::KMZ;
    if (selectedFilter != "Google Earth (*.kmz)") {
        if (selectedFilter == "OpenAir (*.txt)") desiredFormatIndex = AirspaceConverter::OpenAir_Format;
        else if (selectedFilter == "Polish (*.mp)") desiredFormatIndex = AirspaceConverter::Polish;
        else if (selectedFilter == "Garmin map (*.img)") desiredFormatIndex = AirspaceConverter::Garmin;
        else assert(false);
    }

    // Reselect the desired format also in the combo box, he will take care of putting the right extension
    ui->outputFormatComboBox->setCurrentIndex(desiredFormatIndex);
}
