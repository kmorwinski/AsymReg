#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtGui/QMainWindow>

#include <QtCore/QDateTime>
#include <QtCore/QPointer>

class PlotterSettings;
class QFileSystemWatcher;
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
    bool loadPlotterSettings(const QString &fileName, PlotterSettings *sett) const;
    bool savePlotterSettings(const QString &fileName, const PlotterSettings *sett) const;

    PlotterSettings *m_pressureFunctionPlotSettings;
    QAction *m_confNplotPressFuncAction;
    QAction *m_plotPressFuncAction;
    QDateTime m_plotTime;
    QFileSystemWatcher *m_plotWatcher;
    QList<QPointer<SvgViewer> > m_svgViewerList;
};

#endif // MAINWINDOW_H_
