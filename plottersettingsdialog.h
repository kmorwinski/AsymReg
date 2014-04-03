#ifndef PLOTTERSETTINGSDIALOG_H_
#define PLOTTERSETTINGSDIALOG_H_

#include <QtGui/QDialog>

class PlotterSettings;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;

class PlotterSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    PlotterSettingsDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

    void setPlotterSettings(PlotterSettings *settings);

public slots:
    void accept();

private slots:
    void loadValues();
    void pageChanged(const QString &text);
    void resetValues();
    void swapImageDimension();

private:
    void saveValues();
    QGroupBox *setupAxisWidgets(char axis);
    QGroupBox *setupMoreWidgets();
    QGroupBox *setupPageNimgDimWidgets();
    QGroupBox *setupTitlesWidgets();

    PlotterSettings *m_settings;
    QCheckBox *m_pageBorderCheckBox;
    QComboBox *m_fontComboBox;
    QComboBox *m_pageComboBox;
    QDoubleSpinBox *m_xSpinBoxes[2];
    QDoubleSpinBox *m_ySpinBoxes[2];
    QDoubleSpinBox *m_zSpinBoxes[2];
    QGroupBox *m_axisGroupBoxes[3];
    QGroupBox *m_moreGroupBox;
    QLineEdit *m_axisTitles[3];
    QLineEdit *m_titleLines[4];
    QSpinBox *m_heightSpinBox;
    QSpinBox *m_widthSpinBox;
};

#endif // PLOTTERSETTINGSDIALOG_H_
