#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtGui/QMainWindow>

class ContourPlotterSettings;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

private slots:
    void plotPressureFunction(QAction *action = nullptr);
    void runAsymReg();

private:
    ContourPlotterSettings *m_pressureFunctionPlotSettings;
    QAction *m_confNplotPressFuncAction;
    QAction *m_plotPressFuncAction;
};

#endif // MAINWINDOW_H_
