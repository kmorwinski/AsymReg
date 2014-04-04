#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtGui/QMainWindow>

#include <QtCore/QDateTime>
#include <QtCore/QPointer>

class PlotterSettings;
class QActionGroup;
class QFileSystemWatcher;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
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
    void dataSourceChanged(QTableWidgetItem *item);
    void plotPressureFunction(QAction *action = nullptr);
    void runAsymReg();
    void saveDataSource();
    void selectDataSource(QAction *action);
    void showSvgViewer(const QString &path);

private:
    QAction *addDataSourceAction(const QString &fileName);
    void discardDataSource();
    void loadDataSourceToTableWidget();
    int askToSaveDataSource(const QString &fileName);
    void saveDataSource(const QString &fileName, bool reload = true);

    bool loadPlotterSettings(const QString &fileName, PlotterSettings *sett) const;
    bool savePlotterSettings(const QString &fileName, const PlotterSettings *sett) const;

    PlotterSettings *m_pressureFunctionPlotSettings;
    QAction *m_confNplotPressFuncAction;
    QAction *m_plotPressFuncAction;
    QActionGroup *m_dataSourceSelectGroup;
    QDateTime m_plotTime;
    QFileSystemWatcher *m_plotWatcher;
    QList<QPointer<SvgViewer> > m_svgViewerList;
    QPushButton *m_dataSourceSelectButton;
    QPushButton *m_dataSourceSaveButton;
    QTableWidget *m_dataSourceTableWidget;

    bool m_dataSourceChanged;
};

#endif // MAINWINDOW_H_
