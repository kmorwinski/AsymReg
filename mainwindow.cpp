#include "mainwindow.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>

#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QBrush>
#include <QtGui/QCloseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolBar>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>

#include <qjson/qobjecthelper.h>
#include <qjson/parser.h>
#include <qjson/serializer.h>

#include <chrono>
#include <iostream>
#include <fstream>

#include "eigen.h"
#include "interpol.h"
#include "plotter.h"
#include "plottersettings.h"
#include "plottersettingsdialog.h"
#include "svgviewer.h"

// some string we need to seach throughout different places:
#define MW_DATSRC_STR_CLEAR   "Clear List"
#define MW_DATSRC_STR_EMPTY   "Empty"
#define MW_DATSRC_STR_SELECT  "Select &New File"

#define MW_PLTCFG_FILE  "../data/plotconfig.json"

using namespace std;
using namespace Eigen;

static BilinearInterpol *func = nullptr;

MatrixXd zMat;

static bool qaction_lessThan(QAction *ac1, QAction *ac2)
{
    bool ac1IsFile = ac1->isCheckable();
    bool ac2IsFile = ac2->isCheckable();

    if (ac1IsFile == ac2IsFile)
        return (ac1->text() <= ac2->text());

    return (ac1IsFile && !ac2IsFile);
}

MainWindow::MainWindow()
    : m_pressureFunctionPlotSettings(nullptr),
      m_plotConfigChaned(false),
      m_plotTime(QDateTime::currentDateTime()),
      m_dataSourceChanged(false)
{
    setWindowTitle(tr("Main[*] - %1").arg(qApp->applicationName()));

    QAction *quitAction = new QAction(this);
    quitAction->setText(tr("&Quit"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    quitAction->setShortcut(Qt::Key_Escape);
    connect(quitAction, SIGNAL(triggered()),
            this, SLOT(close()));

    QToolBar *toolBar = new QToolBar;
    toolBar->addAction(quitAction);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, toolBar);

    QLabel *infoLabel = new QLabel;
    infoLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setWordWrap(true);
    infoLabel->setText(tr("<br/>"
                          "<b>The Asymptotical Reqularization "
                          "in Computer Tomographie</b><br/>"
                          "<br/>"
                          "<i>Example:</i> Schlieren Imaging<br/>"
                          "<br/>"));

    m_dataSourceTableWidget = new QTableWidget;
    m_dataSourceTableWidget->setRowCount(11);
    m_dataSourceTableWidget->setColumnCount(11);
    connect(m_dataSourceTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(dataSourceChanged(QTableWidgetItem*)));

    QAction *emptyFile = new QAction(this);
    emptyFile->setText(tr(MW_DATSRC_STR_EMPTY));
    emptyFile->setCheckable(true);
    emptyFile->setChecked(true);
    emptyFile->setEnabled(false);

    QAction *clearList = new QAction(this);
    clearList->setText(tr(MW_DATSRC_STR_CLEAR));
    clearList->setToolTip(tr("Closes the selected data source and clears the list of possible files."));
    clearList->setIcon(QIcon::fromTheme("edit-clear"));

    QAction *selectFile = new QAction(this);
    selectFile->setText(tr(MW_DATSRC_STR_SELECT));
    selectFile->setToolTip(tr("Opens a filedialog to select a new source-file."));
    selectFile->setIcon(QIcon::fromTheme("document-open"));

    m_dataSourceSelectGroup = new QActionGroup(this);
    m_dataSourceSelectGroup->addAction(emptyFile);
    m_dataSourceSelectGroup->addAction(clearList);
    m_dataSourceSelectGroup->addAction(selectFile);
    connect(m_dataSourceSelectGroup, SIGNAL(selected(QAction*)),
            this, SLOT(selectDataSource(QAction*)));

    QMenu *dataSourceSelectMenu = new QMenu;
    dataSourceSelectMenu->addActions(m_dataSourceSelectGroup->actions());
    dataSourceSelectMenu->insertSeparator(clearList);

    m_dataSourceSelectButton = new QPushButton;
    m_dataSourceSelectButton->setText(tr("Select &Data Source"));
    m_dataSourceSelectButton->setToolTip(tr("Click here to select a new data source."));
    m_dataSourceSelectButton->setMenu(dataSourceSelectMenu);

    m_dataSourceSaveButton = new QPushButton;
    m_dataSourceSaveButton->setText(tr("&Save Data Source"));
    m_dataSourceSaveButton->setToolTip(tr("Save current data back to selected file."));
    m_dataSourceSaveButton->setEnabled(false);
    connect(m_dataSourceSaveButton, SIGNAL(clicked()),
            this, SLOT(saveDataSource()));

    QPushButton *dataSourcePlotButton = new QPushButton;
    dataSourcePlotButton->setText(tr("Plot Data Source"));
    dataSourcePlotButton->setToolTip(tr("Plots the interpolated data from the current selected data source."));
    connect(dataSourcePlotButton, SIGNAL(clicked()),
            this, SLOT(plotDataSource()));

    QFileInfo plotConfigFile(MW_PLTCFG_FILE);
    QAction *currentPlotConfigAction = new QAction(this);
    currentPlotConfigAction->setText(plotConfigFile.exists() ? plotConfigFile.canonicalFilePath() :
                                                               tr("default"));
    currentPlotConfigAction->setCheckable(true);
    currentPlotConfigAction->setChecked(true);
    currentPlotConfigAction->setEnabled(false);

    QAction *configurePlotConfigAction = new QAction(this);
    configurePlotConfigAction->setText(tr("Change Current Configuration"));
    configurePlotConfigAction->setToolTip(tr("Opens the configuration dialog to adjust your plotting configuration."));
    configurePlotConfigAction->setIcon(QIcon::fromTheme("preferences-other"));

    m_plotConfigSelectGroup = new QActionGroup(this);
    m_plotConfigSelectGroup->addAction(currentPlotConfigAction);
    m_plotConfigSelectGroup->addAction(configurePlotConfigAction);
    connect(m_plotConfigSelectGroup, SIGNAL(selected(QAction*)),
            this, SLOT(selectPlotConfig(QAction*)));

    QMenu *plotConfigSelectMenu = new QMenu;
    plotConfigSelectMenu->addActions(m_plotConfigSelectGroup->actions());
    plotConfigSelectMenu->insertSeparator(configurePlotConfigAction);

    QPushButton *plotConfigSelectButton = new QPushButton;
    plotConfigSelectButton->setText(tr("Select Plotting Configuration"));
    plotConfigSelectButton->setToolTip(tr("Click here to select the configuration for plotting data."));
    plotConfigSelectButton->setMenu(plotConfigSelectMenu);

    QPushButton *runAsymRegButton = new QPushButton;
    runAsymRegButton->setText(tr("Run Asymptotical Regularization"));
    runAsymRegButton->setToolTip(tr("Starts the mathematical part of this programm."));
    runAsymRegButton->setDefault(true);
    runAsymRegButton->setIcon(QIcon::fromTheme("media-playback-start"));
    connect(runAsymRegButton, SIGNAL(clicked(bool)),
            this, SLOT(runAsymReg()));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(infoLabel, 0, 0, 1, 3);
    layout->addWidget(m_dataSourceTableWidget, 1, 0, 5, 2);
    layout->addWidget(m_dataSourceSelectButton, 1, 2, 1, 1);
    layout->addWidget(m_dataSourceSaveButton, 2, 2, 1, 1);
    layout->addWidget(dataSourcePlotButton, 3, 2, 1, 1);
    layout->addWidget(plotConfigSelectButton, 4, 2, 1, 1);
    layout->addWidget(runAsymRegButton, 6, 0, 1, 3);

    setCentralWidget(new QWidget);
    centralWidget()->setLayout(layout);

    QDir dir(QDir::currentPath());
    m_plotWatcher = new QFileSystemWatcher;
    m_plotWatcher->addPath(dir.canonicalPath());
    connect(m_plotWatcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(showSvgViewer(QString)));

    // init matrix/vector:
    zMat.resize(11, 11);
    zMat.setZero();
    loadDataSourceToTableWidget();

    // read window-size, file-lists, etc from settings:
    readSettings();

    // load plotter configuration:
    m_pressureFunctionPlotSettings = new ContourPlotterSettings;
    connect(m_pressureFunctionPlotSettings, SIGNAL(settingsChanged()),
            this, SLOT(changedPlotConfig()));
    selectPlotConfig(currentPlotConfigAction); // read preset file, TODO: Move to readSettings()
}

MainWindow::~MainWindow()
{
    // delete class members:
    delete m_plotWatcher;

    // clean up plot files:
    QDir dir(QDir::currentPath());
    QStringList nameFilters = {"*.svg", "*.png", "*.jpg", "*.jpeg"};
    QStringList images = dir.entryList(nameFilters, QDir::Files);
    auto it = images.constBegin();
    while (it != images.constEnd())
        QFile::remove(*it++);
}

QAction *MainWindow::addDataSourceAction(const QString &fileName, bool checked)
{
    // remove "empty" action:
    QList<QAction *> list = m_dataSourceSelectGroup->actions();
    if (list.size() == 3) { // "Empty", "Select New File" and "Clear List"
        for (int i = 0; i < 3; ++i) {
            if (list.at(i)->text() == tr(MW_DATSRC_STR_EMPTY)) {
                QAction *empty = list.takeAt(i);
                delete empty;
                break;
            }
        }
    }

    // create new QAction:
    QAction *newDataSourceAction = new QAction(this);
    newDataSourceAction->setText(fileName);
    newDataSourceAction->setCheckable(true);
    newDataSourceAction->setActionGroup(m_dataSourceSelectGroup);
    newDataSourceAction->setChecked(checked);

    // sort list to make sure data files come first (start with '/'):
    list << newDataSourceAction;
    qSort(list.begin(), list.end(), qaction_lessThan);

    // remove and re-add all actions:
    QMenu *menu = m_dataSourceSelectButton->menu();
    menu->clear(); // deletes separator, but keeps select-file-action
    menu->addActions(list);
    menu->insertSeparator(list.at(list.size()-2));

    return newDataSourceAction;
}

int MainWindow::askToSaveDataSource(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    // prepare questions and information:
    QString title = tr("Close Main - %1").arg(qApp->applicationName());
    QString text = tr("The data source \"%1\" has been modified.").arg(fileInfo.fileName());
    QString infoText = tr("Do you want to save your changes or discard them?");

    // construct dialog:
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setInformativeText(infoText);
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                              QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setIcon(QMessageBox::Warning);

    // show dialog and evaluate answer:
    auto ret = msgBox.exec();
    if (ret == QMessageBox::Save) {
        QString dtitle = tr("Save File - %1").arg(qApp->applicationName());
        QString proposedFile;
        if (fileInfo.exists())
            proposedFile = fileInfo.canonicalFilePath();
        else
            proposedFile = tr("%1/new_data.csv").arg(QDir("../data").canonicalPath());
        QString filter =  tr("CSV Files(*.csv)");
                filter += ";;" + tr("All Files(*.*)");
        QString dfileName = QFileDialog::getSaveFileName(this,
                                                        dtitle,
                                                        proposedFile,
                                                        filter);
        if (!dfileName.isEmpty())
            saveDataSource(dfileName, false);
        else
            ret = QMessageBox::Cancel;
    } else if (ret == QMessageBox::Discard)
        discardDataSource();

    return ret;
}

int MainWindow::askToSavePlotConfig(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    // prepare questions and information for QMessageBox:
    QString mbTtitle = tr("Close Main - %1").arg(qApp->applicationName());
    QString mbText = tr("The plotter configuration \"%1\" has been modified.").arg(fileInfo.fileName());
    QString mbInfoText = tr("Do you want to save your changes or discard them?");

    // construct dialog:
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(mbTtitle);
    msgBox.setText(mbText);
    msgBox.setInformativeText(mbInfoText);
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                              QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setIcon(QMessageBox::Warning);

    // show dialog and evaluate answer:
    auto ret = msgBox.exec();
    if (ret == QMessageBox::Save) {
        // prepare information for QFileDialog:
        QString fdTitle = tr("Save File - %1").arg(qApp->applicationName());
        QString fdProposedFile;
        if (fileInfo.exists())
            fdProposedFile = fileInfo.canonicalFilePath();
        else
            fdProposedFile = MW_PLTCFG_FILE;
        QString fdFilter =  tr("JSON Files(*.json)");
                fdFilter += ";;" + tr("All Files(*.*)");

        // construct dialog:
        QString newFileName = QFileDialog::getSaveFileName(this,
                                                        fdTitle,
                                                        fdProposedFile,
                                                        fdFilter);

        // evaluate answer (& save):
        if (!newFileName.isEmpty())
            savePlotterSettings(newFileName, m_pressureFunctionPlotSettings);
        else
            ret = QMessageBox::Cancel; // saving cancled, so tell the programm
    }

    return ret; // return answer from QMessageBox
}

void MainWindow::changedPlotConfig()
{
    m_plotConfigChaned = true;
    setWindowModified(m_dataSourceChanged || m_plotConfigChaned);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()) {
        if (m_dataSourceChanged) {
            QAction *dataSource = m_dataSourceSelectGroup->checkedAction();
            int answer = askToSaveDataSource(dataSource->text());
            if (answer == QMessageBox::Cancel) {
                event->ignore();
                return;
            }
        } else if (m_plotConfigChaned) {
            QAction *plotConfig = m_plotConfigSelectGroup->checkedAction();
            int answer = askToSavePlotConfig(plotConfig->text());
            if (answer == QMessageBox::Cancel) {
                event->ignore();
                return;
            }
        }
    }

    if (!m_svgViewerList.isEmpty()) {
        auto it = m_svgViewerList.begin();
        do {
            if (*it) {
                bool closed = (*it)->close();
                if (!closed) {
                    event->ignore();
                    return;
                }
            }
        } while (++it != m_svgViewerList.end());
    }

    // save window-size, fileLists to settings and exit:
    saveSettings();
    event->accept();
}

void MainWindow::dataSourceChanged(QTableWidgetItem *item)
{
    m_dataSourceTableWidget->blockSignals(true); // do not trigger 'itemChanged'-Signal

    bool ok;
    double d = item->data(Qt::DisplayRole).toString().toDouble(&ok);
    int col = m_dataSourceTableWidget->column(item);
    int row = m_dataSourceTableWidget->row(item);

    if (!ok) {
        // restore data:
        const QString v = QString("%L1").arg(zMat.coeff(row,col), 0, 'f');
        item->setData(Qt::DisplayRole, v);
        item->setData(Qt::BackgroundRole, QVariant());

        return;
    }

    // set data & mark as unsaved
    zMat(row, col) = d;
    item->setData(Qt::BackgroundRole, Qt::green);
    m_dataSourceChanged = true; // make sure we know what has changed
    m_dataSourceSaveButton->setEnabled(m_dataSourceChanged);
    setWindowModified(m_dataSourceChanged || m_plotConfigChaned); // show asterik in widowtitle

    m_dataSourceTableWidget->blockSignals(false);
}

void MainWindow::discardDataSource()
{
    if (!isWindowModified() || !m_dataSourceChanged)
        return;

    m_dataSourceChanged = false;
    setWindowModified(m_dataSourceChanged || m_plotConfigChaned);
    m_dataSourceSaveButton->setEnabled(m_dataSourceChanged);
}

void MainWindow::loadDataSourceToTableWidget()
{
    m_dataSourceTableWidget->blockSignals(true); // do not trigger 'itemChanged'-Signal

    int rows = zMat.rows();
    int cols = zMat.cols();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            QTableWidgetItem *item = m_dataSourceTableWidget->item(r, c);
            bool needNew = (item == nullptr);
            if (needNew)
                item = new QTableWidgetItem;
            const QString v = QString("%L1").arg(zMat.coeff(r,c), 0, 'f');
            item->setData(Qt::DisplayRole, v);
            item->setData(Qt::BackgroundRole, QVariant()); // remove green color
            if (needNew)
                m_dataSourceTableWidget->setItem(r, c, item);
        }
    }

    m_dataSourceTableWidget->blockSignals(false);
}

bool MainWindow::loadPlotterSettings(const QString &fileName, PlotterSettings *sett)
{
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    if (!f.isOpen())
        return false;

    bool ok;
    QJson::Parser par;
    QVariant var = par.parse(&f, &ok);
    if (ok)
        QJson::QObjectHelper::qvariant2qobject(var.toMap(), sett);

    return ok;
}

void MainWindow::plotDataSource()
{
    if (func == nullptr)
        return;

    if (m_pressureFunctionPlotSettings == nullptr)
        return;

    int steps = 200;
    VectorXd X, Y;
    X.setLinSpaced(steps, 0., 10.);
    Y = X;

    MatrixXd Z;
    Z.resize(X.size(), Y.size());
    for (int i = 0; i < X.size(); ++i) {
        for (int j = 0; j < Y.size(); ++j)
            Z(i,j) = func->interpol(X(i),Y(j));
    }

    ContourPlotter plotter(m_pressureFunctionPlotSettings, Plotter::Output_SVG_Image);
    plotter.setData(Z);
}

void MainWindow::readSettings()
{
    QAction *dataSource = nullptr;

    QSettings settings;
    settings.beginGroup("Main");
        settings.beginGroup("Window");
            restoreGeometry(settings.value("geometry").toByteArray());
            restoreState(settings.value("windowState").toByteArray());
        settings.endGroup(); // "Window"
        settings.beginGroup("DataSource");
            // restore datasource file actions:
            int size = settings.beginReadArray("source");
            for (int i = 0; i < size; ++i) {
                settings.setArrayIndex(i);
                QString file = settings.value("file").toString();
                bool checked = settings.value("selected").toBool();
                if (QFileInfo(file).exists()) {
                    QAction *ac = addDataSourceAction(file, checked);
                    if (checked)
                        dataSource = ac;
                }
            }
            settings.endArray();
        settings.endGroup(); // "DataSource"
    settings.endGroup(); // "Main"

    if (dataSource != nullptr)
        selectDataSource(dataSource);
}

void MainWindow::runAsymReg()
{
    VectorXd xVec, yVec;
    xVec.setLinSpaced(11, 0., 10.);
    yVec.setLinSpaced(11, 0., 10.);

    auto t1 = std::chrono::high_resolution_clock::now();
    func = new BilinearInterpol(xVec, yVec, zMat);
}

void MainWindow::saveDataSource()
{
    QFileInfo fileInfo(m_dataSourceSelectGroup->checkedAction()->text());
    if (fileInfo.exists())
        saveDataSource(fileInfo.canonicalFilePath());
}

void MainWindow::saveDataSource(const QString &fileName, bool reload)
{
    std::fstream fs;
    fs.open(fileName.toAscii().data(), std::fstream::out | std::fstream::trunc);
    Q_ASSERT(fs.is_open());
    fs << zMat;
    fs.close();

    discardDataSource();
    if (reload) {
        m_dataSourceTableWidget->clearContents();
        loadDataSourceToTableWidget();
    }
}

bool MainWindow::savePlotterSettings(const QString &fileName, const PlotterSettings *sett)
{
    QFile f(fileName);
    f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!f.isOpen())
        return false;

    QVariantMap variantMap = QJson::QObjectHelper::qobject2qvariant(sett);
    bool ok;
    QJson::Serializer ser;
    ser.setIndentMode(QJson::IndentFull);
    QByteArray dat = ser.serialize(variantMap, &ok);
    if (ok)
        f.write(dat);

    return ok;
}

void MainWindow::saveSettings() const
{
    QSettings settings;
    settings.beginGroup("Main");
        settings.beginGroup("Window");
            settings.setValue("geometry", saveGeometry());
            settings.setValue("windowState", saveState());
        settings.endGroup(); // "Window"
        settings.beginGroup("DataSource");
            // save datasource file actions:
            settings.beginWriteArray("source");
            QList<QAction *> list =  m_dataSourceSelectGroup->actions();
            auto it = list.constBegin();
            int arrayIdx = 0;
            do {
                QString text = (*it)->text();
                if ((*it)->isCheckable() && (text != tr(MW_DATSRC_STR_EMPTY))) {
                    settings.setArrayIndex(arrayIdx++);
                    settings.setValue("file", text);
                    settings.setValue("selected", (*it)->isChecked());
                }
            } while (++it != list.constEnd());
            settings.endArray();
        settings.endGroup(); // "DataSource"
    settings.endGroup(); // "Main"
}

void MainWindow::selectDataSource(QAction *action)
{
    Q_ASSERT(action != nullptr);
    static QAction *lastSelectedAction = nullptr;

    if (isWindowModified() && m_dataSourceChanged) {
        QString file;
        if (lastSelectedAction == nullptr)
            file = tr(MW_DATSRC_STR_EMPTY);
        else
            file = lastSelectedAction->text();
        int answer = askToSaveDataSource(file);
        if (answer == QMessageBox::Cancel) {
            lastSelectedAction->setChecked(true);
            return;
        } else if (answer == QMessageBox::Discard) {
            m_dataSourceChanged = false;
            setWindowModified(m_dataSourceChanged || m_plotConfigChaned);
        }
    }

    QString fileName;
    if (action->isCheckable())
        fileName = action->text();
    else if (action->text() == tr(MW_DATSRC_STR_SELECT)) { // Select New File
        QString title = tr("Load File - %1").arg(qApp->applicationName());
        QString proposedFile = QDir("../data").canonicalPath();
        QString filter =  tr("CSV Files(*.csv)");
                filter += ";;" + tr("All Files(*.*)");
        fileName = QFileDialog::getOpenFileName(this,
                                                title,
                                                proposedFile,
                                                filter);
        if (fileName.isEmpty())
            return;

        action = addDataSourceAction(fileName);
    } else { // Clear List
        Q_ASSERT(action->text() == tr(MW_DATSRC_STR_CLEAR)); // assert if not "Clear List"-Action

        // delete all file-actions:
        // QActionGroup takes care of the rest, so we use a
        // const-iterator
        QList<QAction *> all = m_dataSourceSelectGroup->actions();
        auto it = all.constBegin();
        do {
            if ((*it)->isCheckable())
                delete *it;
        } while (++it != all.constEnd());

        action = addDataSourceAction(tr(MW_DATSRC_STR_EMPTY));
        action->setEnabled(false); // Empty is greyed out and not clickable
        action = nullptr; // need nullptr for saving-changed-data logic
    }

    // save for later use:
    lastSelectedAction = action;

    // read data to zMat
    if (!fileName.isEmpty()) {
        std::fstream fs;
        fs.open(fileName.toAscii().data(), std::fstream::in);
        Q_ASSERT(fs.is_open());
        fs >> zMat;
        fs.close();
    } else
        zMat.setZero();

    // update TableWidget:
    loadDataSourceToTableWidget();
}

void MainWindow::selectPlotConfig(QAction *action)
{
    Q_ASSERT(action != nullptr);
    Q_ASSERT(m_pressureFunctionPlotSettings != nullptr);

    if (action->isCheckable()) {
        QFileInfo fi(action->text());
        if (fi.exists()) {
            m_pressureFunctionPlotSettings->blockSignals(true); // do not trigger 'settingsChanged'-signal
            loadPlotterSettings(fi.canonicalFilePath(), m_pressureFunctionPlotSettings);
            m_pressureFunctionPlotSettings->blockSignals(false);
        } else {
            delete m_pressureFunctionPlotSettings;
            m_pressureFunctionPlotSettings = new ContourPlotterSettings;
            connect(m_pressureFunctionPlotSettings, SIGNAL(settingsChanged()),
                    this, SLOT(changedPlotConfig()));
        }

        return;
    }

    PlotterSettingsDialog dialog(this);
    dialog.setPlotterSettings(m_pressureFunctionPlotSettings);
    auto ret = dialog.exec();
    if (ret == QDialog::Rejected)
        return;

    m_pressureFunctionPlotSettings =
            dynamic_cast<ContourPlotterSettings *>(dialog.getPlotterSettings());
}

void MainWindow::showSvgViewer(const QString &path)
{
    // use QDir class to filter all image files in 'path'
    // and sort by modification date:
    QStringList nameFilters = {"*.svg", "*.png", "*.jpg", "*.jpeg"}; // TODO: add more image types
    QStringList images = QDir(path).entryList(nameFilters, QDir::Files, QDir::Time);

    auto it = images.constBegin();
    while (it != images.constEnd()) {
        if (QFileInfo(*it).lastModified() <= m_plotTime)
            break;

        SvgViewer *svgViewer = new SvgViewer(*it++, "a simple image");
        m_svgViewerList << svgViewer;
        svgViewer->show();
    }

    // assume top entry in images-list is the one that was output by dislin:
    m_plotTime = QDateTime::currentDateTime();
}

#include "mainwindow.moc"
