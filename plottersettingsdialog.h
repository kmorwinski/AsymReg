#ifndef PLOTTERSETTINGSDIALOG_H
#define PLOTTERSETTINGSDIALOG_H

#include <QtGui/QDialog>

class PlotterSettings;
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

private:
    void saveValues();

    PlotterSettings *m_settings;
    QSpinBox *m_heightSpinBox;
    QSpinBox *m_widthSpinBox;
};

#endif // PLOTTERSETTINGSDIALOG_H
