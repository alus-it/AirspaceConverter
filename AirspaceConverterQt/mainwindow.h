#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "aboutdialog.h"

#include <chrono>
#include "../src/Airspace.h"


class Waypoint;
class OpenAir;
class QCloseEvent;

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
    virtual void closeEvent(QCloseEvent *event);
    void logMessage(const std::string& message, const bool isError = false);
    void log(const QString& message, const bool isError = false);
    void startBusy();
    void endBusy();

    Ui::MainWindow *ui;
    AboutDialog about;
    std::chrono::high_resolution_clock::time_point startTime;

    std::multimap<int, Airspace> airspaces;
    std::multimap<int, Waypoint*> waypoints;
    OpenAir *openAir;

    std::string outputFile;
    bool outputJustDone;
    bool busy;


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
