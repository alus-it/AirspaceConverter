/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *aboutButton;
    QPushButton *loadAirspaceFileButton;
    QPushButton *unloadAirspacesButton;
    QComboBox *outputFormatComboBox;
    QLabel *label;
    QLabel *label_2;
    QPushButton *loadAirspaceFolderButton;
    QPushButton *loadWaypointFileButton;
    QPushButton *loadWaypointsFolderButton;
    QPushButton *loadRasterMapFileButton;
    QPushButton *loadRasterMapFolderButton;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QSpinBox *numAirspacesLoadedSpinBox;
    QSpinBox *numTerrainMapsLoadedSpinBox;
    QSpinBox *numWaypointsLoadedSpinBox;
    QPushButton *unloadWaypointsButton;
    QPushButton *unloadTerrainMapsButton;
    QProgressBar *progressBar;
    QTextEdit *loggingTextBox;
    QLabel *qnhLabel;
    QDoubleSpinBox *QNHspinBox;
    QLabel *hPaLabel;
    QLabel *defaultTerrainLabel;
    QLabel *meterLabel;
    QSpinBox *defaultAltSpinBox;
    QPushButton *clearLogButton;
    QPushButton *openOutputFolderButton;
    QPushButton *openOutputFileButton;
    QPushButton *closeButton;
    QPushButton *convertButton;
    QPushButton *filterButton;
    QCheckBox *onlyPointsCheckBox;
    QComboBox *openAirCoordinateTypeComboBox;
    QLabel *openAirCoordonateFormatLabel;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(941, 631);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(941, 631));
        MainWindow->setMaximumSize(QSize(941, 631));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/airspaceconverter128.xpm"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        MainWindow->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        aboutButton = new QPushButton(centralWidget);
        aboutButton->setObjectName(QString::fromUtf8("aboutButton"));
        aboutButton->setGeometry(QRect(809, 10, 111, 26));
        aboutButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        loadAirspaceFileButton = new QPushButton(centralWidget);
        loadAirspaceFileButton->setObjectName(QString::fromUtf8("loadAirspaceFileButton"));
        loadAirspaceFileButton->setGeometry(QRect(20, 80, 231, 31));
        loadAirspaceFileButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        unloadAirspacesButton = new QPushButton(centralWidget);
        unloadAirspacesButton->setObjectName(QString::fromUtf8("unloadAirspacesButton"));
        unloadAirspacesButton->setEnabled(false);
        unloadAirspacesButton->setGeometry(QRect(770, 80, 151, 31));
        unloadAirspacesButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        outputFormatComboBox = new QComboBox(centralWidget);
        outputFormatComboBox->addItem(QString());
        outputFormatComboBox->addItem(QString());
        outputFormatComboBox->addItem(QString());
        outputFormatComboBox->addItem(QString());
        outputFormatComboBox->addItem(QString());
        outputFormatComboBox->addItem(QString());
        outputFormatComboBox->setObjectName(QString::fromUtf8("outputFormatComboBox"));
        outputFormatComboBox->setGeometry(QRect(140, 40, 781, 31));
        outputFormatComboBox->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 50, 111, 16));
        label->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(30, 10, 771, 16));
        label_2->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        loadAirspaceFolderButton = new QPushButton(centralWidget);
        loadAirspaceFolderButton->setObjectName(QString::fromUtf8("loadAirspaceFolderButton"));
        loadAirspaceFolderButton->setGeometry(QRect(260, 80, 231, 31));
        loadAirspaceFolderButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        loadWaypointFileButton = new QPushButton(centralWidget);
        loadWaypointFileButton->setObjectName(QString::fromUtf8("loadWaypointFileButton"));
        loadWaypointFileButton->setGeometry(QRect(20, 160, 231, 31));
        loadWaypointFileButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        loadWaypointsFolderButton = new QPushButton(centralWidget);
        loadWaypointsFolderButton->setObjectName(QString::fromUtf8("loadWaypointsFolderButton"));
        loadWaypointsFolderButton->setGeometry(QRect(260, 160, 231, 31));
        loadWaypointsFolderButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        loadRasterMapFileButton = new QPushButton(centralWidget);
        loadRasterMapFileButton->setObjectName(QString::fromUtf8("loadRasterMapFileButton"));
        loadRasterMapFileButton->setGeometry(QRect(20, 120, 231, 31));
        loadRasterMapFileButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        loadRasterMapFolderButton = new QPushButton(centralWidget);
        loadRasterMapFolderButton->setObjectName(QString::fromUtf8("loadRasterMapFolderButton"));
        loadRasterMapFolderButton->setGeometry(QRect(260, 120, 231, 31));
        loadRasterMapFolderButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(500, 90, 141, 16));
        label_3->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(500, 130, 141, 16));
        label_4->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(500, 170, 141, 16));
        label_5->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        numAirspacesLoadedSpinBox = new QSpinBox(centralWidget);
        numAirspacesLoadedSpinBox->setObjectName(QString::fromUtf8("numAirspacesLoadedSpinBox"));
        numAirspacesLoadedSpinBox->setEnabled(false);
        numAirspacesLoadedSpinBox->setGeometry(QRect(640, 80, 111, 31));
        numAirspacesLoadedSpinBox->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        numAirspacesLoadedSpinBox->setReadOnly(true);
        numAirspacesLoadedSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        numAirspacesLoadedSpinBox->setMaximum(9999999);
        numTerrainMapsLoadedSpinBox = new QSpinBox(centralWidget);
        numTerrainMapsLoadedSpinBox->setObjectName(QString::fromUtf8("numTerrainMapsLoadedSpinBox"));
        numTerrainMapsLoadedSpinBox->setEnabled(false);
        numTerrainMapsLoadedSpinBox->setGeometry(QRect(640, 120, 111, 31));
        numTerrainMapsLoadedSpinBox->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        numTerrainMapsLoadedSpinBox->setReadOnly(true);
        numTerrainMapsLoadedSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        numTerrainMapsLoadedSpinBox->setMaximum(999999);
        numWaypointsLoadedSpinBox = new QSpinBox(centralWidget);
        numWaypointsLoadedSpinBox->setObjectName(QString::fromUtf8("numWaypointsLoadedSpinBox"));
        numWaypointsLoadedSpinBox->setEnabled(false);
        numWaypointsLoadedSpinBox->setGeometry(QRect(640, 160, 111, 31));
        numWaypointsLoadedSpinBox->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        numWaypointsLoadedSpinBox->setReadOnly(true);
        numWaypointsLoadedSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        numWaypointsLoadedSpinBox->setMaximum(99999999);
        unloadWaypointsButton = new QPushButton(centralWidget);
        unloadWaypointsButton->setObjectName(QString::fromUtf8("unloadWaypointsButton"));
        unloadWaypointsButton->setEnabled(false);
        unloadWaypointsButton->setGeometry(QRect(770, 160, 151, 31));
        unloadWaypointsButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        unloadTerrainMapsButton = new QPushButton(centralWidget);
        unloadTerrainMapsButton->setObjectName(QString::fromUtf8("unloadTerrainMapsButton"));
        unloadTerrainMapsButton->setEnabled(false);
        unloadTerrainMapsButton->setGeometry(QRect(770, 120, 151, 31));
        unloadTerrainMapsButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(20, 340, 901, 31));
        progressBar->setMaximum(100);
        progressBar->setValue(0);
        progressBar->setTextVisible(false);
        loggingTextBox = new QTextEdit(centralWidget);
        loggingTextBox->setObjectName(QString::fromUtf8("loggingTextBox"));
        loggingTextBox->setGeometry(QRect(20, 430, 901, 181));
        loggingTextBox->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        loggingTextBox->setReadOnly(true);
        qnhLabel = new QLabel(centralWidget);
        qnhLabel->setObjectName(QString::fromUtf8("qnhLabel"));
        qnhLabel->setGeometry(QRect(20, 210, 141, 16));
        qnhLabel->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QNHspinBox = new QDoubleSpinBox(centralWidget);
        QNHspinBox->setObjectName(QString::fromUtf8("QNHspinBox"));
        QNHspinBox->setGeometry(QRect(170, 200, 111, 31));
        QNHspinBox->setMinimum(800.000000000000000);
        QNHspinBox->setMaximum(1300.000000000000000);
        QNHspinBox->setValue(1013.250000000000000);
        hPaLabel = new QLabel(centralWidget);
        hPaLabel->setObjectName(QString::fromUtf8("hPaLabel"));
        hPaLabel->setGeometry(QRect(290, 210, 461, 20));
        hPaLabel->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        defaultTerrainLabel = new QLabel(centralWidget);
        defaultTerrainLabel->setObjectName(QString::fromUtf8("defaultTerrainLabel"));
        defaultTerrainLabel->setGeometry(QRect(20, 250, 141, 16));
        defaultTerrainLabel->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        meterLabel = new QLabel(centralWidget);
        meterLabel->setObjectName(QString::fromUtf8("meterLabel"));
        meterLabel->setGeometry(QRect(290, 250, 31, 20));
        meterLabel->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        defaultAltSpinBox = new QSpinBox(centralWidget);
        defaultAltSpinBox->setObjectName(QString::fromUtf8("defaultAltSpinBox"));
        defaultAltSpinBox->setGeometry(QRect(170, 240, 111, 31));
        defaultAltSpinBox->setMinimum(-100);
        defaultAltSpinBox->setMaximum(10000);
        defaultAltSpinBox->setValue(30);
        clearLogButton = new QPushButton(centralWidget);
        clearLogButton->setObjectName(QString::fromUtf8("clearLogButton"));
        clearLogButton->setGeometry(QRect(590, 380, 161, 31));
        openOutputFolderButton = new QPushButton(centralWidget);
        openOutputFolderButton->setObjectName(QString::fromUtf8("openOutputFolderButton"));
        openOutputFolderButton->setEnabled(false);
        openOutputFolderButton->setGeometry(QRect(190, 380, 161, 31));
        openOutputFileButton = new QPushButton(centralWidget);
        openOutputFileButton->setObjectName(QString::fromUtf8("openOutputFileButton"));
        openOutputFileButton->setEnabled(false);
        openOutputFileButton->setGeometry(QRect(20, 380, 161, 31));
        closeButton = new QPushButton(centralWidget);
        closeButton->setObjectName(QString::fromUtf8("closeButton"));
        closeButton->setGeometry(QRect(760, 380, 161, 31));
        convertButton = new QPushButton(centralWidget);
        convertButton->setObjectName(QString::fromUtf8("convertButton"));
        convertButton->setEnabled(false);
        convertButton->setGeometry(QRect(20, 280, 901, 41));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        convertButton->setFont(font);
        filterButton = new QPushButton(centralWidget);
        filterButton->setObjectName(QString::fromUtf8("filterButton"));
        filterButton->setEnabled(false);
        filterButton->setGeometry(QRect(770, 200, 151, 31));
        filterButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        onlyPointsCheckBox = new QCheckBox(centralWidget);
        onlyPointsCheckBox->setObjectName(QString::fromUtf8("onlyPointsCheckBox"));
        onlyPointsCheckBox->setEnabled(false);
        onlyPointsCheckBox->setGeometry(QRect(330, 250, 221, 21));
        openAirCoordinateTypeComboBox = new QComboBox(centralWidget);
        openAirCoordinateTypeComboBox->addItem(QString());
        openAirCoordinateTypeComboBox->addItem(QString());
        openAirCoordinateTypeComboBox->addItem(QString());
        openAirCoordinateTypeComboBox->setObjectName(QString::fromUtf8("openAirCoordinateTypeComboBox"));
        openAirCoordinateTypeComboBox->setEnabled(false);
        openAirCoordinateTypeComboBox->setGeometry(QRect(770, 240, 151, 31));
        openAirCoordonateFormatLabel = new QLabel(centralWidget);
        openAirCoordonateFormatLabel->setObjectName(QString::fromUtf8("openAirCoordonateFormatLabel"));
        openAirCoordonateFormatLabel->setEnabled(false);
        openAirCoordonateFormatLabel->setGeometry(QRect(560, 250, 201, 20));
        openAirCoordonateFormatLabel->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        MainWindow->setCentralWidget(centralWidget);
        QWidget::setTabOrder(aboutButton, outputFormatComboBox);
        QWidget::setTabOrder(outputFormatComboBox, loadAirspaceFileButton);
        QWidget::setTabOrder(loadAirspaceFileButton, loadAirspaceFolderButton);
        QWidget::setTabOrder(loadAirspaceFolderButton, numAirspacesLoadedSpinBox);
        QWidget::setTabOrder(numAirspacesLoadedSpinBox, unloadAirspacesButton);
        QWidget::setTabOrder(unloadAirspacesButton, loadWaypointFileButton);
        QWidget::setTabOrder(loadWaypointFileButton, loadWaypointsFolderButton);
        QWidget::setTabOrder(loadWaypointsFolderButton, numWaypointsLoadedSpinBox);
        QWidget::setTabOrder(numWaypointsLoadedSpinBox, unloadWaypointsButton);
        QWidget::setTabOrder(unloadWaypointsButton, loadRasterMapFileButton);
        QWidget::setTabOrder(loadRasterMapFileButton, loadRasterMapFolderButton);
        QWidget::setTabOrder(loadRasterMapFolderButton, numTerrainMapsLoadedSpinBox);
        QWidget::setTabOrder(numTerrainMapsLoadedSpinBox, unloadTerrainMapsButton);
        QWidget::setTabOrder(unloadTerrainMapsButton, QNHspinBox);
        QWidget::setTabOrder(QNHspinBox, defaultAltSpinBox);
        QWidget::setTabOrder(defaultAltSpinBox, filterButton);
        QWidget::setTabOrder(filterButton, convertButton);
        QWidget::setTabOrder(convertButton, openOutputFileButton);
        QWidget::setTabOrder(openOutputFileButton, openOutputFolderButton);
        QWidget::setTabOrder(openOutputFolderButton, clearLogButton);
        QWidget::setTabOrder(clearLogButton, closeButton);
        QWidget::setTabOrder(closeButton, loggingTextBox);

        retranslateUi(MainWindow);
        QObject::connect(closeButton, SIGNAL(clicked()), MainWindow, SLOT(close()));
        QObject::connect(clearLogButton, SIGNAL(clicked()), loggingTextBox, SLOT(clear()));

        openAirCoordinateTypeComboBox->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "AirspaceConverter", nullptr));
#if QT_CONFIG(tooltip)
        aboutButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Click this button to get more info about this software.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        aboutButton->setText(QCoreApplication::translate("MainWindow", "About...", nullptr));
#if QT_CONFIG(tooltip)
        loadAirspaceFileButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Choose one or more input airspace file(s). The input files can be in the openAIP, OpenAir or Google Earth KML/KMZ formats.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        loadAirspaceFileButton->setText(QCoreApplication::translate("MainWindow", "Load airspace input file(s)", nullptr));
#if QT_CONFIG(tooltip)
        unloadAirspacesButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Clears all the airspace definitions currently loaded in memory.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        unloadAirspacesButton->setText(QCoreApplication::translate("MainWindow", "Unload airspaces", nullptr));
        outputFormatComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "KMZ Google Earth", nullptr));
        outputFormatComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "OpenAir (only airspace)", nullptr));
        outputFormatComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "CUP SeeYou (only waypoints)", nullptr));
        outputFormatComboBox->setItemText(3, QCoreApplication::translate("MainWindow", "CSV LittleNavMap (only waypoints)", nullptr));
        outputFormatComboBox->setItemText(4, QCoreApplication::translate("MainWindow", "Polish format for cGPSmapper", nullptr));
        outputFormatComboBox->setItemText(5, QCoreApplication::translate("MainWindow", "IMG file for Garmin devices", nullptr));

#if QT_CONFIG(tooltip)
        outputFormatComboBox->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Select the desired output format.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label->setText(QCoreApplication::translate("MainWindow", "Output format:", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "AirspaceConverter is an experimental software. The authors decline any responsability. Use only at your own risk.", nullptr));
#if QT_CONFIG(tooltip)
        loadAirspaceFolderButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Choose a folder with input airspace files. All the contained files, in openAIP, OpenAir and Google Earth KML/KMZ will be attempted to load. Beware to don't reload the just converted files.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        loadAirspaceFolderButton->setText(QCoreApplication::translate("MainWindow", "Load airspace files from folder", nullptr));
#if QT_CONFIG(tooltip)
        loadWaypointFileButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Load one or more waypoints files in the SeeYou (.CUP) format.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        loadWaypointFileButton->setText(QCoreApplication::translate("MainWindow", "Load waypoints input file(s)", nullptr));
#if QT_CONFIG(tooltip)
        loadWaypointsFolderButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Load all the waypoints files in the SeeYou (.CUP) format contained in a folder.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        loadWaypointsFolderButton->setText(QCoreApplication::translate("MainWindow", "Load waypoints files from folder", nullptr));
#if QT_CONFIG(tooltip)
        loadRasterMapFileButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Load one or multiple terrain raster maps (.DEM) files. Terrain maps are used to get the AMSL altitude of AGL altitudes.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        loadRasterMapFileButton->setText(QCoreApplication::translate("MainWindow", "Load raster terrain map file(s)", nullptr));
#if QT_CONFIG(tooltip)
        loadRasterMapFolderButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Load all terrain raster maps (.DEM) files from a directory. Terrain maps are used to get the AMSL altitude of AGL altitudes.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        loadRasterMapFolderButton->setText(QCoreApplication::translate("MainWindow", "Load raster maps from folder", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Airspaces loaded:", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Terrain map loaded:", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Waypoints loaded:", nullptr));
#if QT_CONFIG(tooltip)
        numAirspacesLoadedSpinBox->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>The number of airspace definitions currently loaded.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        numTerrainMapsLoadedSpinBox->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>The number of terrain raster maps currently loaded.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        numWaypointsLoadedSpinBox->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>The number of waypoints currently loaded.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        unloadWaypointsButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Clears all the waypoints currently loaded in memory.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        unloadWaypointsButton->setText(QCoreApplication::translate("MainWindow", "Unload waypoints", nullptr));
#if QT_CONFIG(tooltip)
        unloadTerrainMapsButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Clears all the terrain raster maps currently loaded in memory.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        unloadTerrainMapsButton->setText(QCoreApplication::translate("MainWindow", "Unload maps", nullptr));
        qnhLabel->setText(QCoreApplication::translate("MainWindow", "QNH:", nullptr));
#if QT_CONFIG(tooltip)
        QNHspinBox->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>If desired, only before to load airspace, set here a specific QNH to be used to calculate the AMSL height of flight levels. This is used only to make Google Earth files and applied only when reading input files.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        hPaLabel->setText(QCoreApplication::translate("MainWindow", "hPa   (used to calculate altitude of flight levels, only during input)", nullptr));
        defaultTerrainLabel->setText(QCoreApplication::translate("MainWindow", "Default terrain alt:", nullptr));
        meterLabel->setText(QCoreApplication::translate("MainWindow", "m", nullptr));
#if QT_CONFIG(tooltip)
        defaultAltSpinBox->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Set here the default terrain altitude in meters that will be used for the points not covered by the loaded terrain raster maps. Used only to make Google Earth files.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        clearLogButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Clears all the text from the below message log.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        clearLogButton->setText(QCoreApplication::translate("MainWindow", "Clear log", nullptr));
#if QT_CONFIG(tooltip)
        openOutputFolderButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Opens the folder with the just created output file.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        openOutputFolderButton->setText(QCoreApplication::translate("MainWindow", "Open output folder", nullptr));
#if QT_CONFIG(tooltip)
        openOutputFileButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Opens the just created output file with its predefined software.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        openOutputFileButton->setText(QCoreApplication::translate("MainWindow", "Open output file", nullptr));
#if QT_CONFIG(tooltip)
        closeButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Terminates AirspaceConverter.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        closeButton->setText(QCoreApplication::translate("MainWindow", "Close", nullptr));
#if QT_CONFIG(tooltip)
        convertButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:400;\">Starts the conversion process.</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        convertButton->setText(QCoreApplication::translate("MainWindow", "Convert", nullptr));
#if QT_CONFIG(tooltip)
        filterButton->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Opens a dialog to define a filtering area. When the filter is applied all airspace and waypoints outside the defined area will be unloaded. So, when converting, the resulting output will be only in the selected area.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        filterButton->setText(QCoreApplication::translate("MainWindow", "Filter on area", nullptr));
#if QT_CONFIG(tooltip)
        onlyPointsCheckBox->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Only when writing to OpenAir this option forces the converter to don't use OpenAir arc (DA) and circle (DC) definititions but only points (DP). This option can be useful when comparing to other OpenAir files made with only points, otherwise it is better to keep it disabled and take advantage of the use of arcs and circles, resulting in smaller OpenAir files.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        onlyPointsCheckBox->setText(QCoreApplication::translate("MainWindow", "Use only OpenAir DP points", nullptr));
        openAirCoordinateTypeComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "DD:MM.MMM", nullptr));
        openAirCoordinateTypeComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "DD:MM:SS", nullptr));
        openAirCoordinateTypeComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "Auto", nullptr));

#if QT_CONFIG(tooltip)
        openAirCoordinateTypeComboBox->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Choose here the output format for coordinates in OpenAir. Coordinates expressed as DD:MM:SS (with seconds) are more compact and readable while DD:MM.MMM (with decimal minutes) are more accurate. &quot;Auto&quot; will automatically use the more convenient format every time: with seconds when the coordinate matches a specific second value or in decimal minutes in the other cases to remain accurate.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        openAirCoordonateFormatLabel->setText(QCoreApplication::translate("MainWindow", "OpenAir coordiante format:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
