/********************************************************************************
** Form generated from reading UI file 'aboutdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QPushButton *OKpushButton;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *versionLabel;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *icon;
    QLabel *compileDateLabel;
    QLabel *label_5;
    QLabel *newVersionLabel;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->setEnabled(true);
        AboutDialog->resize(380, 230);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AboutDialog->sizePolicy().hasHeightForWidth());
        AboutDialog->setSizePolicy(sizePolicy);
        AboutDialog->setMinimumSize(QSize(380, 230));
        AboutDialog->setMaximumSize(QSize(380, 230));
        AboutDialog->setContextMenuPolicy(Qt::NoContextMenu);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/airspaceconverter128.xpm"), QSize(), QIcon::Normal, QIcon::Off);
        AboutDialog->setWindowIcon(icon1);
        AboutDialog->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        OKpushButton = new QPushButton(AboutDialog);
        OKpushButton->setObjectName(QString::fromUtf8("OKpushButton"));
        OKpushButton->setGeometry(QRect(270, 180, 95, 31));
        label = new QLabel(AboutDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(110, 10, 251, 16));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        label_2 = new QLabel(AboutDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(110, 30, 61, 16));
        label_3 = new QLabel(AboutDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(110, 70, 221, 21));
        label_4 = new QLabel(AboutDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(20, 130, 291, 16));
        versionLabel = new QLabel(AboutDialog);
        versionLabel->setObjectName(QString::fromUtf8("versionLabel"));
        versionLabel->setGeometry(QRect(180, 30, 181, 16));
        label_7 = new QLabel(AboutDialog);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(20, 180, 241, 16));
        label_7->setTextFormat(Qt::RichText);
        label_7->setOpenExternalLinks(true);
        label_7->setTextInteractionFlags(Qt::TextBrowserInteraction);
        label_8 = new QLabel(AboutDialog);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(20, 200, 241, 16));
        label_8->setTextFormat(Qt::RichText);
        label_8->setOpenExternalLinks(true);
        label_8->setTextInteractionFlags(Qt::TextBrowserInteraction);
        icon = new QLabel(AboutDialog);
        icon->setObjectName(QString::fromUtf8("icon"));
        icon->setGeometry(QRect(20, 20, 64, 64));
        icon->setPixmap(QPixmap(QString::fromUtf8(":/icons/airspaceconverter128.xpm")));
        icon->setScaledContents(true);
        compileDateLabel = new QLabel(AboutDialog);
        compileDateLabel->setObjectName(QString::fromUtf8("compileDateLabel"));
        compileDateLabel->setGeometry(QRect(110, 50, 261, 16));
        label_5 = new QLabel(AboutDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(110, 150, 181, 20));
        newVersionLabel = new QLabel(AboutDialog);
        newVersionLabel->setObjectName(QString::fromUtf8("newVersionLabel"));
        newVersionLabel->setGeometry(QRect(20, 100, 341, 21));

        retranslateUi(AboutDialog);
        QObject::connect(OKpushButton, SIGNAL(clicked()), AboutDialog, SLOT(close()));

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QCoreApplication::translate("AboutDialog", "About AirspaceConverter", nullptr));
        OKpushButton->setText(QCoreApplication::translate("AboutDialog", "&OK", nullptr));
        label->setText(QCoreApplication::translate("AboutDialog", "AirspaceConverter", nullptr));
        label_2->setText(QCoreApplication::translate("AboutDialog", "Version:", nullptr));
        label_3->setText(QCoreApplication::translate("AboutDialog", "Copyright (C) 2016", nullptr));
#if QT_CONFIG(whatsthis)
        label_4->setWhatsThis(QCoreApplication::translate("AboutDialog", "<html><head/><body><p>AirspaceConverter author</p></body></html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        label_4->setText(QCoreApplication::translate("AboutDialog", "Written by:    Alberto Realis-Luc", nullptr));
#if QT_CONFIG(tooltip)
        versionLabel->setToolTip(QCoreApplication::translate("AboutDialog", "<html><head/><body><p>This is the version of the current installed AirspaceConverter.</p><p>Check the website for newer versions.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        versionLabel->setWhatsThis(QCoreApplication::translate("AboutDialog", "<html><head/><body><p><br/></p></body></html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        versionLabel->setText(QString());
#if QT_CONFIG(tooltip)
        label_7->setToolTip(QCoreApplication::translate("AboutDialog", "<html><head/><body><p>For more info click this link to visit AirspaceConverter website.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label_7->setText(QCoreApplication::translate("AboutDialog", "<a href=\"https://www.alus.it/AirspaceConverter/\">www.alus.it/AirspaceConverter</a>", nullptr));
#if QT_CONFIG(tooltip)
        label_8->setToolTip(QCoreApplication::translate("AboutDialog", "<html><head/><body><p>For any comment about AirspaceConverter write to this e-mail.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label_8->setText(QCoreApplication::translate("AboutDialog", "<a href=\"mailto:info@alus.it\">info@alus.it</a>", nullptr));
#if QT_CONFIG(whatsthis)
        icon->setWhatsThis(QCoreApplication::translate("AboutDialog", "<html><head/><body><p><br/></p></body></html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        icon->setText(QString());
#if QT_CONFIG(tooltip)
        compileDateLabel->setToolTip(QCoreApplication::translate("AboutDialog", "<html><head/><body><p>This is exactly when the current version was compiled.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        compileDateLabel->setWhatsThis(QCoreApplication::translate("AboutDialog", "<html><head/><body><p><br/></p></body></html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        compileDateLabel->setText(QString());
#if QT_CONFIG(whatsthis)
        label_5->setWhatsThis(QCoreApplication::translate("AboutDialog", "<html><head/><body><p>AirspaceConverter author</p></body></html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        label_5->setText(QCoreApplication::translate("AboutDialog", "Valerio Messina", nullptr));
#if QT_CONFIG(tooltip)
        newVersionLabel->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        newVersionLabel->setWhatsThis(QCoreApplication::translate("AboutDialog", "<html><head/><body><p><br/></p></body></html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        newVersionLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
