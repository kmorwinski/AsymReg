#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtGui/QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

private slots:
    void plotPressureFunction();
    void runAsymReg();
};

#endif // MAINWINDOW_H_
