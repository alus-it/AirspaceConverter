#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "aboutdialog.h"

#include "../src/Airspace.h"


class Waypoint;
class OpenAir;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void logMessage(const std::string&, const bool isError = false);


    Ui::MainWindow *ui;
    AboutDialog about;

    std::multimap<int, Airspace> airspaces;
    std::multimap<int, Waypoint*> waypoints;
    OpenAir *openAir;

    std::string outputFile;


private slots:
    void on_aboutButton_clicked();
    void on_outputFormatComboBox_currentIndexChanged(int index);
    void on_loadAirspaceFileButton_clicked();
    void on_loadAirspaceFolderButton_clicked();
    void on_unloadAirspacesButton_clicked();
    void on_loadWaypointFileButton_clicked();
    void on_loadWaypointsFolderButton_clicked();
    void on_unloadWaypointsButton_clicked();
    void on_loadRasterMapFileButton_clicked();
    void on_loadRasterMapFolderButton_clicked();
    void on_unloadTerrainMapsButton_clicked();
    void on_openOutputFileButton_clicked();
    void on_openOutputFolderButton_clicked();
    void on_convertButton_clicked();
    void on_chooseOutputFileButton_clicked();
};

#endif // MAINWINDOW_H
