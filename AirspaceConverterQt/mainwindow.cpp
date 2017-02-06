#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDesktopServices>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <cassert>
#include "AirspaceConverter.h"
#include "CUPreader.h"
#include "OpenAIPreader.h"
#include "KMLwriter.h"
#include "PFMwriter.h"
#include "OpenAir.h"
#include "Waypoint.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    openAir(nullptr),
    busy(false)
{
    // Set the logging function (to write in the logging texbox)
    std::function<void(const std::string&, const bool)> func = std::bind(&MainWindow::logMessage, this, std::placeholders::_1, std::placeholders::_2);
    AirspaceConverter::SetLogMessageFuntion(func);

    // Build OpenAir module
    openAir = new OpenAir(airspaces);
    assert(openAir != nullptr);

    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
    delete openAir;

    // Clear waypoints
    for (const std::pair<const int, Waypoint*>& wpt : waypoints) delete wpt.second;
    waypoints.clear();
}

void MainWindow::logMessage(const std::string& message, const bool isError) {
    log(QString::fromStdString(message), isError);
}

void MainWindow::log(const QString& message, const bool isError) {
    ui->loggingTextBox->setTextColor(isError ? "red" : "black");
    ui->loggingTextBox->append(message);
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

void MainWindow::endBusy(bool conversionJustDone) {
    // Stop the timer
    if (busy) {
        const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
        logMessage(std::string(boost::str(boost::format("Execution time: %1f sec.") %elapsedTimeSec)));
    }

    // Re-enable all specifically
    ui->outputFormatComboBox->setEnabled(true);
    ui->loadAirspaceFileButton->setEnabled(true);
    ui->loadAirspaceFolderButton->setEnabled(true);
    ui->unloadAirspacesButton->setEnabled(!airspaces.empty());
    ui->loadWaypointFileButton->setEnabled(true);
    ui->loadWaypointsFolderButton->setEnabled(true);
    ui->unloadWaypointsButton->setEnabled(!waypoints.empty());
    ui->loadRasterMapFileButton->setEnabled(true);
    ui->loadRasterMapFolderButton->setEnabled(true);
    ui->unloadTerrainMapsButton->setEnabled(KMLwriter::GetNumOfRasterMaps()>0);
    ui->defaultAltSpinBox->setEnabled(true);
    ui->QNHspinBox->setEnabled(true);
    ui->chooseOutputFileButton->setEnabled(true);
    ui->convertButton->setEnabled(!airspaces.empty());
    ui->openOutputFileButton->setEnabled(conversionJustDone);
    ui->openOutputFolderButton->setEnabled(conversionJustDone);
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
    if(outputFile.empty()) return;
    boost::filesystem::path path(outputFile);
    switch(newExtIdx) {
        case AirspaceConverter::KMZ:
            path.replace_extension(".kmz");
            break;
        case AirspaceConverter::OpenAir:
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
}

void MainWindow::readSingleAirspaceFile(const std::string& inputFile, const std::string& ext) {
    bool redOk(false);
    if(boost::iequals(ext, ".txt")) redOk = openAir->ReadFile(inputFile);
    else if(boost::iequals(ext, ".aip")) redOk = OpenAIPreader::ReadFile(inputFile, airspaces);
    else return;
    if (outputFile.empty() && redOk) {
        outputFile = inputFile;
        updateOutputFileExtension(ui->outputFormatComboBox->currentIndex());
    }
}

void MainWindow::on_loadAirspaceFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Airspace files"), QDir::currentPath(), tr("All airspace files (*.txt *.aip);;OpenAir (*.txt);;OpenAIP (*.aip);;") );
    if(filenames.empty() || openAir == nullptr) return;

    startBusy();

    // Load all the files
    for(const auto& file : filenames) {
        const std::string inputFile(file.toStdString());
        readSingleAirspaceFile(inputFile, boost::filesystem::path(inputFile).extension().string());
    }

    // Set the numer of airspaces loaded in its spinBox
    ui->numAirspacesLoadedSpinBox->setValue((int)airspaces.size());

    endBusy();
}

void MainWindow::on_loadAirspaceFolderButton_clicked() {
    boost::filesystem::path root(QFileDialog::getExistingDirectory(this, tr("Open airspace directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks).toStdString());
    if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen

    startBusy();

    for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
        if (!boost::filesystem::is_regular_file(*it)) continue;
        readSingleAirspaceFile(it->path().string(), it->path().extension().string());
    }

    // Set the numer of airspaces loaded in its spinBox
    ui->numAirspacesLoadedSpinBox->setValue((int)airspaces.size());

    endBusy();
}

void MainWindow::on_unloadAirspacesButton_clicked() {
    airspaces.clear();
    ui->numAirspacesLoadedSpinBox->setValue(0);
    endBusy();
}

void MainWindow::on_loadWaypointFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Waypoints files"), QDir::currentPath(), tr("CUP files(*.cup);;") );
    if(filenames.empty()) return;

    startBusy();

    // Load all the files
    for(const auto& file : filenames) CUPreader::ReadFile(file.toStdString(), waypoints);

    // Set the numer of waypoints loaded in its spinBox
    ui->numWaypointsLoadedSpinBox->setValue((int)waypoints.size());

    endBusy();
}

void MainWindow::on_loadWaypointsFolderButton_clicked() {
    boost::filesystem::path root(QFileDialog::getExistingDirectory(this, tr("Open waypoints directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks).toStdString());
    if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen

    startBusy();

    for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
        if (!boost::filesystem::is_regular_file(*it)) continue;
        std::string ext(it->path().extension().string());
        if(boost::iequals(ext, ".cup")) CUPreader::ReadFile(it->path().string(), waypoints);
    }

    // Set the numer of waypoints loaded in its spinBox
    ui->numWaypointsLoadedSpinBox->setValue((int)waypoints.size());

    endBusy();
}

void MainWindow::on_unloadWaypointsButton_clicked() {
    waypoints.clear();
    ui->numWaypointsLoadedSpinBox->setValue(0);
}

void MainWindow::on_loadRasterMapFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Terrain raster map files"), QDir::currentPath(), tr("DEM files (*.dem);;"));
    if(filenames.empty()) return;

    startBusy();

    // Load terrain maps
    for(const auto& mapFile : filenames) KMLwriter::AddTerrainMap(mapFile.toStdString());

    // Set the numer of terrain raster map loaded in its spinBox
    ui->numTerrainMapsLoadedSpinBox->setValue(KMLwriter::GetNumOfRasterMaps());

    endBusy();
}

void MainWindow::on_loadRasterMapFolderButton_clicked() {
    boost::filesystem::path root(QFileDialog::getExistingDirectory(this, tr("Open raster terrain maps directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks).toStdString());
    if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) return; //this should never happen

    startBusy();

    for (boost::filesystem::directory_iterator it(root), endit; it != endit; ++it) {
        if (!boost::filesystem::is_regular_file(*it)) continue;
        std::string ext(it->path().extension().string());
        if(boost::iequals(ext, ".dem")) KMLwriter::AddTerrainMap(it->path().string());
    }

    // Set the numer of terrain raster map loaded in its spinBox
    ui->numTerrainMapsLoadedSpinBox->setValue(KMLwriter::GetNumOfRasterMaps());

    endBusy();
}

void MainWindow::on_unloadTerrainMapsButton_clicked() {
    KMLwriter::ClearTerrainMaps();
    ui->numTerrainMapsLoadedSpinBox->setValue(0);
}

void MainWindow::on_convertButton_clicked()
{
    startBusy();

    // Set QNH
    Altitude::SetQNH(ui->QNHspinBox->value());

    // Set default terrain altitude
    KMLwriter::SetDefaultTerrainAltitude(ui->defaultAltSpinBox->value());

    bool outputJustDone(false);

    switch(ui->outputFormatComboBox->currentIndex()) {
        case AirspaceConverter::KMZ:
            {
                if(KMLwriter::GetNumOfRasterMaps() == 0) log("Warning: no terrain map loaded, using default terrain height for all applicable AGL points.", true);

                // Make KML file
                outputJustDone = KMLwriter().WriteFile(outputFile, airspaces, waypoints);
            }
            break;
        case AirspaceConverter::OpenAir:
            if (openAir != nullptr) outputJustDone = openAir->WriteFile(outputFile);
            break;
        case AirspaceConverter::Polish:
            outputJustDone = PFMwriter().WriteFile(outputFile, airspaces);
            break;
        case AirspaceConverter::Garmin:
            {
                outputJustDone = false;

                // First make Polish file
                const std::string polishFile(boost::filesystem::path(outputFile).replace_extension(".mp").string());
                if(!PFMwriter().WriteFile(polishFile, airspaces)) break;

                // Then call cGPSmapper
                logMessage(std::string(boost::str(boost::format("Invoking cGPSmapper to make: %1s") %outputFile)));

                //TODO: add arguments to create files also for other software like Garmin BaseCamp
                const std::string cmd(boost::str(boost::format("./cGPSmapper/cgpsmapper-static %1s -o %2s") %polishFile %outputFile));

                if(system(cmd.c_str()) == EXIT_SUCCESS) {
                    std::remove(polishFile.c_str()); // Delete polish file
                    outputJustDone = true;
                } else log("ERROR: returned by cGPSmapper.", true);
            }
            break;
        default:
            assert(false);
    }

    endBusy(outputJustDone);
}

void MainWindow::on_openOutputFileButton_clicked() {    
    //TODO: QDesktopServices???
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(outputFile)));
}

void MainWindow::on_openOutputFolderButton_clicked() {
    //TODO: QDesktopServices???

    //boost::filesystem::path(outputFile)

    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(outputFile)));
}

void MainWindow::on_chooseOutputFileButton_clicked() {
    // Prepare dialog to ask for output file, will be without extension if not manually typed by the user
    QString selectedFilter; // this will conatin the selected type by the user in the dialog
    outputFile = QFileDialog::getSaveFileName(this, tr("Output file"), QDir::currentPath(), tr("Google Earth (*.kmz);;OpenAir (*.txt);;Polish (*.mp);;Garmin map (*.img)"), &selectedFilter).toStdString();

    // Determine which format is now selected
    int desiredFormatIndex = AirspaceConverter::KMZ;
    if (selectedFilter != "Google Earth (*.kmz)") {
        if (selectedFilter == "OpenAir (*.txt)") desiredFormatIndex = AirspaceConverter::OpenAir;
        else if (selectedFilter == "Polish (*.mp)") desiredFormatIndex = AirspaceConverter::Polish;
        else if (selectedFilter == "Garmin map (*.img)") desiredFormatIndex = AirspaceConverter::Garmin;
        else assert(false);
    }

    // Reselect the desired format also in the combo box, he will take care of putting the right extension
    ui->outputFormatComboBox->setCurrentIndex(desiredFormatIndex);
}
