#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include "../src/AirspaceConverter.h"
#include "../src/CUPreader.h"
#include "../src/OpenAIPreader.h"
#include "../src/KMLwriter.h"
#include "../src/PFMwriter.h"
#include "../src/OpenAir.h"
#include "../src/Waypoint.h"
#include <chrono>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <cassert>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    openAir(nullptr)
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
    ui->loggingTextBox->setTextColor(isError ? "red" : "black");
    ui->loggingTextBox->append(QString::fromStdString(message));
}

void MainWindow::on_aboutButton_clicked() {
    about.show();
}

void MainWindow::on_outputFormatComboBox_currentIndexChanged(int index) {
    assert(index >= AirspaceConverter::KMZ && index < AirspaceConverter::NumOfOutputTypes);

    //TODO: set the output filename
}

void MainWindow::on_loadAirspaceFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Airspace files"), QDir::currentPath(), tr("All airspace files (*.txt *.aip);;OpenAir (*.txt);;OpenAIP (*.aip);;") );
    if(filenames.empty() || openAir == nullptr) return;

    // Load all the files
    for(const auto& file : filenames) {
        std::string inputFile(file.toStdString());
        std::string ext(boost::filesystem::path(inputFile).extension().string());
        if(boost::iequals(ext, ".txt")) openAir->ReadFile(inputFile);
        else if(boost::iequals(ext, ".aip")) OpenAIPreader::ReadFile(inputFile, airspaces);
        else assert(false);
    }

    // Set the numer of airspaces loaded in its spinBox
    ui->numAirspacesLoadedSpinBox->setValue(airspaces.size());

    // Eventually set the name of output file
    if(outputFile.empty())
        outputFile = boost::filesystem::path(filenames.front().toStdString()).replace_extension(".kmz").string();
    //TODO: set proper extension...
}

void MainWindow::on_loadAirspaceFolderButton_clicked() {
    logMessage("TO BE DONE!!!!", true);

}

void MainWindow::on_unloadAirspacesButton_clicked() {
    airspaces.clear();
    ui->numAirspacesLoadedSpinBox->setValue(0);
}

void MainWindow::on_loadWaypointFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Waypoints files"), QDir::currentPath(), tr("CUP files(*.cup);;") );
    if(filenames.empty()) return;

    // Load all the files
    for(const auto& file : filenames) CUPreader::ReadFile(file.toStdString(), waypoints);

    // Set the numer of waypoints loaded in its spinBox
    ui->numWaypointsLoadedSpinBox->setValue(waypoints.size());
}

void MainWindow::on_loadWaypointsFolderButton_clicked() {
}

void MainWindow::on_unloadWaypointsButton_clicked() {
    waypoints.clear();
    ui->numWaypointsLoadedSpinBox->setValue(0);
}

void MainWindow::on_loadRasterMapFileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Terrain raster map files"), QDir::currentPath(), tr("DEM files (*.dem);;"));
    if(filenames.empty()) return;

    // Load terrain maps
    for(const auto& mapFile : filenames) KMLwriter::AddTerrainMap(mapFile.toStdString());

    // Set the numer of terrain raster map loaded in its spinBox
    ui->numTerrainMapsLoadedSpinBox->setValue(KMLwriter::GetNumOfRasterMaps());
}

void MainWindow::on_loadRasterMapFolderButton_clicked() {
}

void MainWindow::on_unloadTerrainMapsButton_clicked() {
    KMLwriter::ClearTerrainMaps();
    ui->numTerrainMapsLoadedSpinBox->setValue(0);
}

void MainWindow::on_convertButton_clicked()
{
    // Start the timer
    const auto startTime = std::chrono::high_resolution_clock::now();

    // Set QNH
    Altitude::SetQNH(ui->QNHspinBox->value());

    // Set default terrain altitude
    KMLwriter::SetDefaultTerrainAltitude(ui->defaultAltSpinBox->value());


    switch(ui->outputFormatComboBox->currentIndex()) {
        case AirspaceConverter::KMZ:
            {
                if(KMLwriter::GetNumOfRasterMaps() == 0) logMessage("Warning: no terrain map loaded, using default terrain height for all applicable AGL points.");

                // Make KML file
                KMLwriter().WriteFile(outputFile, airspaces, waypoints);
            }
            break;
        case AirspaceConverter::OpenAir:
            if (openAir != nullptr) openAir->WriteFile(outputFile);
            break;
        case AirspaceConverter::Polish:
            PFMwriter().WriteFile(outputFile, airspaces);
            break;
        case AirspaceConverter::Garmin:
            {
                // First make Polish file
                const std::string polishFile(boost::filesystem::path(outputFile).replace_extension(".mp").string());
                if(!PFMwriter().WriteFile(polishFile, airspaces)) break;

                // Then call cGPSmapper
                std::cout << "Invoking cGPSmapper to make: " << outputFile << std::endl << std::endl;

                //TODO: add arguments to create files also for other software like Garmin BaseCamp
                const std::string cmd(boost::str(boost::format("./cGPSmapper/cgpsmapper-static %1s -o %2s") %polishFile %outputFile));

                if(system(cmd.c_str()) == EXIT_SUCCESS) {
                    std::remove(polishFile.c_str()); // Delete polish file
                } else std::cerr << std::endl << "ERROR: cGPSmapper returned an error." << std::endl;
            }
            break;
        default:
            assert(false);
            break;
    }


    // Stop the timer
    const double elapsedTimeSec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1e6;
    std::cout<<"Total execution time: " << elapsedTimeSec << " sec." << std::endl << std::endl;
}

void MainWindow::on_openOutputFileButton_clicked() {
}

void MainWindow::on_openOutputFolderButton_clicked() {

}

void MainWindow::on_chooseOutputFileButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "./", tr("Airsapce files (*.png *.jpg *.bmp)"));
}
