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
    int diffLatestVersion;
    if (AirspaceConverter::CheckForNewVersion(diffLatestVersion)) {
        if (diffLatestVersion == 0) ui->newVersionLabel->setText("Congratulations: this is the latest version.");
        else if (diffLatestVersion > 0) {
            QPalette palette = ui->newVersionLabel->palette();
            palette.setColor(ui->newVersionLabel->foregroundRole(), Qt::red);
            ui->newVersionLabel->setPalette(palette);
            ui->newVersionLabel->setText("A new version is available! Check our website.");
        }
        else ui->newVersionLabel->setText("This version is not yet released.");
    }
}

AboutDialog::~AboutDialog() {
    delete ui;
}
