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

#pragma once
#include <QMainWindow>
#include <QFutureWatcher>
#include "aboutdialog.h"
#include "limitsdialog.h"
#include <chrono>

class AirspaceConverter;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    virtual void closeEvent(QCloseEvent *event);
    void postMessage(const std::string& message, const bool isError = false);
    void startBusy();
    void laodAirspacesThread();
    void refreshUI();

    Ui::MainWindow *ui;
    AboutDialog about;
    LimitsDialog filter;
    AirspaceConverter* converter;
    std::chrono::high_resolution_clock::time_point startTime;
    bool busy;
    QFutureWatcher<void> watcher;
    QString suggestedInputDir;
    bool outputFileInsertedViaDialog;

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
    void on_filterButton_clicked();
    void on_openOutputFileButton_clicked();
    void on_openOutputFolderButton_clicked();
    void on_convertButton_clicked();
    void endBusy();
    void logMessage(const QString& message, const bool& isError = false);
    void applyFilter(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon);

signals:
    void messagePosted(const QString& message, const bool& isError = false);
};
