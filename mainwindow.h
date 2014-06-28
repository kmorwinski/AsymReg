#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtGui/QMainWindow>

#include <QtCore/QDateTime>
#include <QtCore/QPointer>
#include <QtCore/QStack>

class DataSourceTableWidget;
class PlotterSettings;
class QActionGroup;
class QComboBox;
class QDoubleSpinBox;
class QFileSystemWatcher;
class QPushButton;
class QSpinBox;
class QTableWidgetItem;
class QToolButton;
class SvgViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

    QMenu *createPopupMenu() override;

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
    void closeAllPlots();
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
    int askToSaveDataSource(const QString &fileName = QString());
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

    // members for run configuration:
    QComboBox *m_runSolverSelectComboBox;
    QDoubleSpinBox *m_runEulerStepSpinBox;
    QPushButton *m_runAsymRegButton;
    QSpinBox *m_runEulerIterationSpinBox;
    QSpinBox *m_runGridSizeSpinBox;

    // members for plotter configuration handling:
    PlotterSettings *m_pressureFunctionPlotSettings;
    QActionGroup *m_plotConfigSelectGroup;
    QPushButton *m_plotConfigSelectButton;
    QStack<QString> m_plotImageTitleStack;
    bool m_plotConfigChaned;

    // members for data source handling:
    DataSourceTableWidget *m_dataSourceTableWidget;
    QActionGroup *m_dataSourceSelectGroup;
    QPushButton *m_dataSourceSelectButton;
    QPushButton *m_dataSourceSaveButton;
    bool m_dataSourceChanged;

    // members for viewing plots:
    QAction *m_autoPlotDataSrcAction;
    QAction *m_autoPlotDataRegAction;
    QAction *m_closeAllPlotsAction;
    QDateTime m_plotTime;
    QFileSystemWatcher *m_plotWatcher;
    QList<QPointer<SvgViewer> > m_svgViewerList;
    QToolButton *m_autoPlotToolButton;

    // other:
    QAction *m_autoRunAction;
};

#endif // MAINWINDOW_H_
