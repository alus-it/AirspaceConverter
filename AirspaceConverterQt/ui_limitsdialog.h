/********************************************************************************
** Form generated from reading UI file 'limitsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LIMITSDIALOG_H
#define UI_LIMITSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_LimitsDialog
{
public:
    QDialogButtonBox *buttonBox;
    QFrame *frame;
    QLabel *label;
    QDoubleSpinBox *topLatSpinBox;
    QDoubleSpinBox *bottomLatSpinBox;
    QDoubleSpinBox *rightLonSpinBox;
    QDoubleSpinBox *leftLonSpinBox;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;

    void setupUi(QDialog *LimitsDialog)
    {
        if (LimitsDialog->objectName().isEmpty())
            LimitsDialog->setObjectName(QString::fromUtf8("LimitsDialog"));
        LimitsDialog->resize(469, 314);
        buttonBox = new QDialogButtonBox(LimitsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(360, 10, 91, 51));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        frame = new QFrame(LimitsDialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(130, 70, 211, 161));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        label = new QLabel(frame);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(60, 70, 101, 16));
        topLatSpinBox = new QDoubleSpinBox(LimitsDialog);
        topLatSpinBox->setObjectName(QString::fromUtf8("topLatSpinBox"));
        topLatSpinBox->setGeometry(QRect(250, 40, 91, 24));
        topLatSpinBox->setDecimals(3);
        topLatSpinBox->setMinimum(-90.000000000000000);
        topLatSpinBox->setMaximum(90.000000000000000);
        topLatSpinBox->setValue(90.000000000000000);
        bottomLatSpinBox = new QDoubleSpinBox(LimitsDialog);
        bottomLatSpinBox->setObjectName(QString::fromUtf8("bottomLatSpinBox"));
        bottomLatSpinBox->setGeometry(QRect(250, 240, 91, 24));
        bottomLatSpinBox->setDecimals(3);
        bottomLatSpinBox->setMinimum(-90.000000000000000);
        bottomLatSpinBox->setMaximum(90.000000000000000);
        bottomLatSpinBox->setValue(-90.000000000000000);
        rightLonSpinBox = new QDoubleSpinBox(LimitsDialog);
        rightLonSpinBox->setObjectName(QString::fromUtf8("rightLonSpinBox"));
        rightLonSpinBox->setGeometry(QRect(350, 130, 101, 24));
        rightLonSpinBox->setDecimals(3);
        rightLonSpinBox->setMinimum(-180.000000000000000);
        rightLonSpinBox->setMaximum(180.000000000000000);
        rightLonSpinBox->setValue(180.000000000000000);
        leftLonSpinBox = new QDoubleSpinBox(LimitsDialog);
        leftLonSpinBox->setObjectName(QString::fromUtf8("leftLonSpinBox"));
        leftLonSpinBox->setGeometry(QRect(10, 130, 101, 24));
        leftLonSpinBox->setDecimals(3);
        leftLonSpinBox->setMinimum(-180.000000000000000);
        leftLonSpinBox->setMaximum(180.000000000000000);
        leftLonSpinBox->setValue(-180.000000000000000);
        label_2 = new QLabel(LimitsDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(130, 40, 111, 16));
        label_3 = new QLabel(LimitsDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 110, 111, 16));
        label_4 = new QLabel(LimitsDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(350, 110, 111, 16));
        label_5 = new QLabel(LimitsDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(130, 240, 111, 16));
        label_6 = new QLabel(LimitsDialog);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 10, 341, 16));
        label_7 = new QLabel(LimitsDialog);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 270, 451, 20));
        QWidget::setTabOrder(topLatSpinBox, bottomLatSpinBox);
        QWidget::setTabOrder(bottomLatSpinBox, leftLonSpinBox);
        QWidget::setTabOrder(leftLonSpinBox, rightLonSpinBox);

        retranslateUi(LimitsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), LimitsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), LimitsDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(LimitsDialog);
    } // setupUi

    void retranslateUi(QDialog *LimitsDialog)
    {
        LimitsDialog->setWindowTitle(QCoreApplication::translate("LimitsDialog", "Insert filtering area limits", nullptr));
#if QT_CONFIG(tooltip)
        buttonBox->setToolTip(QCoreApplication::translate("LimitsDialog", "Confirm and apply filter selection with \"OK\" or abort with \"Cancel\".", nullptr));
#endif // QT_CONFIG(tooltip)
        label->setText(QCoreApplication::translate("LimitsDialog", "Selected area", nullptr));
#if QT_CONFIG(tooltip)
        topLatSpinBox->setToolTip(QCoreApplication::translate("LimitsDialog", "Enter here the northern latitude of your selection", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        bottomLatSpinBox->setToolTip(QCoreApplication::translate("LimitsDialog", "Enter here the southern latitude of your selection", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        rightLonSpinBox->setToolTip(QCoreApplication::translate("LimitsDialog", "Enter here the eastern longitude of your selection", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        leftLonSpinBox->setToolTip(QCoreApplication::translate("LimitsDialog", "Enter here the western longitude of your selection", nullptr));
#endif // QT_CONFIG(tooltip)
        label_2->setText(QCoreApplication::translate("LimitsDialog", "North latitude:", nullptr));
        label_3->setText(QCoreApplication::translate("LimitsDialog", "West longitude:", nullptr));
        label_4->setText(QCoreApplication::translate("LimitsDialog", "East longitude:", nullptr));
        label_5->setText(QCoreApplication::translate("LimitsDialog", "South latitude:", nullptr));
        label_6->setText(QCoreApplication::translate("LimitsDialog", "Insert the bounds of the area to filter on.", nullptr));
        label_7->setText(QCoreApplication::translate("LimitsDialog", "Negative latitudes are to South, negative longitudes are to West.", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LimitsDialog: public Ui_LimitsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LIMITSDIALOG_H
