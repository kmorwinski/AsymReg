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
    // private slots for data source handling:
    void dataSourceChanged(QTableWidgetItem *item);
    void saveDataSource();
    void selectDataSource(QAction *action);

    void plotPressureFunction(QAction *action = nullptr);
    void runAsymReg();

    // private slots for viewing plots:
    void showSvgViewer(const QString &path);

private:
    // private functions for data source handling:
    QAction *addDataSourceAction(const QString &fileName, bool checked = true);
    int askToSaveDataSource(const QString &fileName);
    void discardDataSource();
    void loadDataSourceToTableWidget();
    void saveDataSource(const QString &fileName, bool reload = true);

    // read and save Plotter-Settings from/to JSON files:
    bool loadPlotterSettings(const QString &fileName, PlotterSettings *sett) const;
    bool savePlotterSettings(const QString &fileName, const PlotterSettings *sett) const;

    // read and save settings, file-lists & windows-size:
    void readSettings();
    void saveSettings() const;

    PlotterSettings *m_pressureFunctionPlotSettings;
    QAction *m_confNplotPressFuncAction;
    QAction *m_plotPressFuncAction;

    // members for data source handling:
    QActionGroup *m_dataSourceSelectGroup;
    QPushButton *m_dataSourceSelectButton;
    QPushButton *m_dataSourceSaveButton;
    QTableWidget *m_dataSourceTableWidget;
    bool m_dataSourceChanged;

    // members for viewing plots:
    QList<QPointer<SvgViewer> > m_svgViewerList;
    QDateTime m_plotTime;
    QFileSystemWatcher *m_plotWatcher;
};

#endif // MAINWINDOW_H_
