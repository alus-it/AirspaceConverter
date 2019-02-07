//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Authors     : Alberto Realis-Luc <alberto.realisluc@gmail.com>
//               Valerio Messina <efa@iol.it>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016-2019 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    setlocale(LC_ALL, "C"); // so std::stod recognize dot as decimal separator
    MainWindow w;
    w.show();
    return a.exec();
}
