//============================================================================
// AirspaceConverter
// Since       : 26/11/2017
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "limitsdialog.h"
#include "ui_limitsdialog.h"
#include <QMessageBox>
#include "Geometry.h"

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
    if (Geometry::Limits(ui->topLatSpinBox->value(), ui->bottomLatSpinBox->value(), ui->leftLonSpinBox->value(), ui->rightLonSpinBox->value()).IsValid())
        emit validLimitsSet(ui->topLatSpinBox->value(), ui->bottomLatSpinBox->value(), ui->leftLonSpinBox->value(), ui->rightLonSpinBox->value());
    else QMessageBox::warning(this, "Invalid limits", tr("The entered limit bounds are not valid."), QMessageBox::Ok);
}
