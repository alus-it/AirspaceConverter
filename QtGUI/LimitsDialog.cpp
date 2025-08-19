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

#include "LimitsDialog.hpp"
#include "ui_LimitsDialog.h"
#include <QMessageBox>
#include "Geometry.hpp"
#include "Altitude.hpp"

LimitsDialog::LimitsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LimitsDialog),
    validLimits(false) {
    ui->setupUi(this);
}

LimitsDialog::~LimitsDialog() {
    delete ui;
}

void LimitsDialog::on_buttonBox_accepted() {
    bool validAreaLimits(false);
    if (ui->filterOnPositionCheckBox->isChecked()) {
        validAreaLimits = Geometry::Limits(ui->topLatSpinBox->value(), ui->bottomLatSpinBox->value(), ui->leftLonSpinBox->value(), ui->rightLonSpinBox->value()).IsValid();
        if (!validAreaLimits) QMessageBox::warning(this, "Invalid area limits", tr("The entered area limit bounds are not valid."), QMessageBox::Ok);
    }
    bool validAltLimits(false);
    Altitude ceiling;
    Altitude floor;
    if (ui->filterOnAltitudeCheckBox->isChecked()) {
        if (ui->unlimitedTopAltitudeCheckBox->isChecked()) ceiling.SetUnlimited();
        else ceiling = Altitude(double(ui->hiAltLimitSpinBox->value()), ui->hiAltLimitUnitComboBox->currentIndex() == 1, true);
        floor = Altitude(double(ui->lowAltLimitSpinBox->value()), ui->lowAltLimitUnitComboBox->currentIndex() == 1, true);
        validAltLimits = (ceiling >= floor);
        if (!validAltLimits) QMessageBox::warning(this, "Invalid altitude limits", tr("The entered altitude limits are not valid."), QMessageBox::Ok);
    }
    if (validAreaLimits || validAltLimits)
        emit validLimitsSet(validAreaLimits, ui->topLatSpinBox->value(), ui->bottomLatSpinBox->value(), ui->leftLonSpinBox->value(), ui->rightLonSpinBox->value(), validAltLimits, floor, ceiling);
}

void LimitsDialog::on_filterOnPositionCheckBox_stateChanged(int checked) {
    ui->topLatSpinBox->setEnabled(checked);
    ui->bottomLatSpinBox->setEnabled(checked);
    ui->leftLonSpinBox->setEnabled(checked);
    ui->rightLonSpinBox->setEnabled(checked);
}

void LimitsDialog::on_filterOnAltitudeCheckBox_stateChanged(int checked) {
    ui->hiAltLimitSpinBox->setEnabled(checked ? !ui->unlimitedTopAltitudeCheckBox->isChecked() : checked);
    ui->hiAltLimitUnitComboBox->setEnabled(checked ? !ui->unlimitedTopAltitudeCheckBox->isChecked() : checked);
    ui->lowAltLimitSpinBox->setEnabled(checked);
    ui->lowAltLimitUnitComboBox->setEnabled(checked);
    ui->unlimitedTopAltitudeCheckBox->setEnabled(checked);
}

void LimitsDialog::on_unlimitedTopAltitudeCheckBox_stateChanged(int checked) {
    ui->hiAltLimitSpinBox->setEnabled(!checked);
    ui->hiAltLimitUnitComboBox->setEnabled(!checked);
}
