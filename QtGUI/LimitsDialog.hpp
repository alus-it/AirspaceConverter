//============================================================================
// AirspaceConverter
// Since       : 26/11/2017
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2017 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <QDialog>

namespace Ui {
class LimitsDialog;
}

class Altitude;

class LimitsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LimitsDialog(QWidget *parent = nullptr);
    ~LimitsDialog();

private slots:
    void on_buttonBox_accepted();
    void on_filterOnAltitudeCheckBox_stateChanged(int checked);
    void on_filterOnPositionCheckBox_stateChanged(int checked);
    void on_unlimitedTopAltitudeCheckBox_stateChanged(int checked);

private:
    Ui::LimitsDialog *ui;
    bool validLimits;

signals:
    void validPositionLimitsSet(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon);
    void validAltitudeLimitsSet(const Altitude& floor, const Altitude& ceil);
};
