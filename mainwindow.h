#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtGui/QMainWindow>

#include <QtCore/QPointer>

class QFileSystemWatcher;
class ContourPlotterSettings;
class SvgViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void plotPressureFunction(QAction *action = nullptr);
    void runAsymReg();
    void showSvgViewer(const QString &path);

private:
    ContourPlotterSettings *m_pressureFunctionPlotSettings;
    QAction *m_confNplotPressFuncAction;
    QAction *m_plotPressFuncAction;
    QFileSystemWatcher *m_plotWatcher;
    QList<QPointer<SvgViewer> > m_svgViewerList;
};

#endif // MAINWINDOW_H_
