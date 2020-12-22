//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : https://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2021 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "AirspaceConverter.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    ui->versionLabel->setText(VERSION);
    ui->compileDateLabel->setText(QString("Compiled on %1 at %2").arg(__DATE__).arg(__TIME__));
}

AboutDialog::~AboutDialog() {
    delete ui;
}
