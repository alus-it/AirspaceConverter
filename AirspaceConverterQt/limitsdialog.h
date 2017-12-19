//============================================================================
// AirspaceConverter
// Since       : 26/11/2017
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2018 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once
#include <QDialog>

namespace Ui {
class LimitsDialog;
}

class LimitsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LimitsDialog(QWidget *parent = nullptr);
    ~LimitsDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::LimitsDialog *ui;
    bool validLimits;

signals:
    void validLimitsSet(const double& topLat, const double& bottomLat, const double& leftLon, const double& rightLon);
};
