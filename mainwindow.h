#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtGui/QMainWindow>

#include <QtCore/QDateTime>
#include <QtCore/QPointer>
#include <QtCore/QStack>

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
    void plotDataSource();

    // private slots for plotter configuration handling:
    void selectPlotConfig(QAction *action);
    void changedPlotConfig();

    // Asymptotical Regularization:
    void runAsymReg();

    // private slots for viewing plots:
    void showSvgViewer(const QString &path);

private:
    // functions to shorten long file names:
    QString elidedFileName(const QString &fileName) const;
    enum {
        FileNameElideMode = Qt::ElideLeft,
        FileNameElideSize = 200
    };

    // private functions for data source handling:
    QAction *addDataSourceAction(const QString &fileName, bool checked = true);
    int askToSaveDataSource(const QString &fileName);
    void discardDataSource();
    void loadDataSourceToTableWidget();
    void saveDataSource(const QString &fileName, bool reload = true);

    // private functions for plotter configuration handling:
    int askToSavePlotConfig(const QString &fileName);

    // read and save Plotter-Settings from/to JSON files:
    static bool loadPlotterSettings(const QString &fileName, PlotterSettings *sett);
    static bool savePlotterSettings(const QString &fileName, const PlotterSettings *sett);

    // read and save settings, file-lists & windows-size:
    void readSettings();
    void saveSettings() const;

    // members for plotter configuration handling:
    PlotterSettings *m_pressureFunctionPlotSettings;
    QActionGroup *m_plotConfigSelectGroup;
    QStack<QString> m_plotImageTitleStack;
    bool m_plotConfigChaned;

    // members for data source handling:
    QActionGroup *m_dataSourceSelectGroup;
    QPushButton *m_dataSourceSelectButton;
    QPushButton *m_dataSourceSaveButton;
    QTableWidget *m_dataSourceTableWidget;
    bool m_dataSourceChanged;

    // members for viewing plots:
    QAction *m_autoPlotAction;
    QList<QPointer<SvgViewer> > m_svgViewerList;
    QDateTime m_plotTime;
    QFileSystemWatcher *m_plotWatcher;
};

#endif // MAINWINDOW_H_
