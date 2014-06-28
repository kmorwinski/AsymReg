#include "mainwindow.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>

#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QCloseEvent>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFileDialog>
#include <QtGui/QFontMetrics>
#include <QtGui/QFormLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStatusBar>
#include <QtGui/QScrollArea>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QToolTip>
#include <QtGui/QVBoxLayout>

#include <qjson/qobjecthelper.h>
#include <qjson/parser.h>
#include <qjson/serializer.h>

#include <iostream>
#include <fstream>

#include "3rdparty/kiconutils.h"

#include "asymreg.h"
#include "constants.h"
#include "datasourcetablewidget.h"
#include "duration.h"
#include "plotter.h"
#include "plottersettings.h"
#include "plottersettingsdialog.h"
#include "svgviewer.h"

// some string we need to seach throughout different places:
#define MW_DATSRC_STR_CLEAR   "Clear List"
#define MW_DATSRC_STR_EMPTY   "Empty"
#define MW_DATSRC_STR_SELECT  "Select &New File"

#define MW_PLTCFG_FILE  "../data/plotconfig.json"

// private variables:
static Eigen::MatrixXd zMat; // TODO: move to MainWindow or AsymReg class

// private functions:
static bool qaction_lessThan(QAction *ac1, QAction *ac2)
{
    bool ac1IsFile = ac1->isCheckable();
    bool ac2IsFile = ac2->isCheckable();

    if (ac1IsFile == ac2IsFile)
        return (ac1->text() <= ac2->text());

    return (ac1IsFile && !ac2IsFile);
}

/**
 * Derived QMenu class to show QAction's tooltips.
 * Tooltips are normally not shown for QAction elements beeing part of a QMenu.
 * This behaviour is intended in Qt4 toolkit, but we want those tooltips to show up.
 * It is needed e.g. to show the whole filename of a file-action.
 *
 * Class does not do much, it just overrides the protected event() function to catch
 * QHelpEvent's and show or hide QAction's tooltip.
 * Taken from here: http://qt-project.org/faq/answer/how_can_i_add_tooltips_to_actions_in_menus
 */
class Menu : public QMenu
{
public:
    bool event(QEvent *ev)
    {
        Q_ASSERT(ev != nullptr);
        const QHelpEvent *helpEvent = static_cast<QHelpEvent *>(ev);
        if (helpEvent->type() == QEvent::ToolTip) {
            QAction *active = activeAction();
            if (active != nullptr) { // <-- trying to prevent a hard to reproduce crash
                                     // when mainwindow gets hidden but menu just pops up
                // call QToolTip::showText on that QAction's tooltip:
                QToolTip::showText(helpEvent->globalPos(),
                                   active->toolTip());
            }
        } else
            QToolTip::hideText();

        return QMenu::event(ev);
    }
};

MainWindow::MainWindow()
    : m_plotConfigChaned(false),
      m_plotTime(QDateTime::currentDateTime()),
      m_dataSourceChanged(false)
{
    std::cout << "Using Optimisations: "
#ifdef _OPENMP
              << "OpenMP, "
#endif
              << Eigen::SimdInstructionSetsInUse() << std::endl;

    /* window properties: */
    setWindowTitle(tr("Main[*] - %1").arg(qApp->applicationName()));

    /* toolbar: */
    QAction *quitAction = new QAction(this);
    quitAction->setText(tr("&Quit"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    quitAction->setShortcut(Qt::Key_Escape);
    connect(quitAction, SIGNAL(triggered()),
            this, SLOT(close()));

    m_autoPlotDataSrcAction = new QAction(this);
    m_autoPlotDataSrcAction->setText(tr("Source Data"));
    m_autoPlotDataSrcAction->setToolTip(tr("Plot data source whenever it is generated."));
    m_autoPlotDataSrcAction->setCheckable(true); // state will be set in readSettings(), default "true"

    m_autoPlotDataRegAction = new QAction(this);
    m_autoPlotDataRegAction->setText(tr("Regularized Data"));
    m_autoPlotDataRegAction->setToolTip(tr("Plot result after regularization has finished."));
    m_autoPlotDataRegAction->setCheckable(true); // state will be set in readSettings(), default "true"

    Menu *autoPlotMenu = new Menu;
    autoPlotMenu->addAction(m_autoPlotDataSrcAction);
    autoPlotMenu->addAction(m_autoPlotDataRegAction);

    m_autoPlotToolButton = new QToolButton;
    m_autoPlotToolButton->setText(tr("Auto Plot"));
    m_autoPlotToolButton->setIcon(KIconUtils::addOverlay(QIcon::fromTheme("image-x-generic"),
                                                         QIcon::fromTheme("media-seek-forward"),
                                                         Qt::BottomLeftCorner));
    m_autoPlotToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // button does not honor toolbar's settings
    m_autoPlotToolButton->setCheckable(true); // state will be set in readSettings(), default "true"
    m_autoPlotToolButton->setMenu(autoPlotMenu);

    m_closeAllPlotsAction = new QAction(this);
    m_closeAllPlotsAction->setText(tr("Close All Plots"));
    //m_closeAllPlotsAction->setToolTip(); // TODO
    m_closeAllPlotsAction->setIcon(KIconUtils::addOverlay(QIcon::fromTheme("image-x-generic"),
                                                          QIcon::fromTheme("window-close"),
                                                          Qt::BottomLeftCorner));
    m_closeAllPlotsAction->setDisabled(true); // will be enabled after plotting
    connect(m_closeAllPlotsAction, SIGNAL(triggered()),
            this, SLOT(closeAllPlots()));

    m_autoRunAction = new QAction(this);
    m_autoRunAction->setText(tr("Auto Run"));
    m_autoRunAction->setToolTip(tr("Autostarts the Asymptotical Regularization on program startup."));
    m_autoRunAction->setIcon(QIcon::fromTheme("media-seek-forward"));
    m_autoRunAction->setCheckable(true); // state will be set in readSettings(), default "false"

    QToolBar *toolBar = new QToolBar;
    toolBar->addAction(quitAction);
    QAction *autoPlotAction = toolBar->addWidget(m_autoPlotToolButton);
    toolBar->addAction(m_closeAllPlotsAction);
    toolBar->addAction(m_autoRunAction);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->insertSeparator(autoPlotAction);  // separator between quit and autoplot
    toolBar->insertSeparator(m_autoRunAction); // separator between close-all-plots and autorun
    toolBar->setMovable(false);
    toolBar->setObjectName("mainToolBar"); // shutdown warning from QSettings
    toolBar->setWindowTitle(tr("Toolbar")); // name shown in popup menu
    addToolBar(Qt::TopToolBarArea, toolBar);

    /* info label: */
    QLabel *infoLabel = new QLabel;
    infoLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setWordWrap(true);
    infoLabel->setText(tr("<br/>"
                          "<b>The Asymptotical Reqularization "
                          "in Computer Tomographie</b><br/>"
                          "<br/>"
                          "<i>Example:</i> Schlieren Imaging<br/>"
                          "<br/><br/>"));

    /* run configuration widget: */
    QFormLayout *runConfigLayoutLeft = new QFormLayout;  // we use the whole space by using 2 layouts,...
    QFormLayout *runConfigLayoutRight = new QFormLayout; // ... one on the left & one on the right side

    m_runGridSizeSpinBox = new QSpinBox; // value is set in readSettings()
    m_runGridSizeSpinBox->setSingleStep(10);
    m_runGridSizeSpinBox->setMaximum(250);
    m_runGridSizeSpinBox->setDisabled(true); // has no function yet
    runConfigLayoutLeft->addRow(tr("system grid size:"), m_runGridSizeSpinBox);

    QLabel *odeLabel = new QLabel;
    odeLabel->setText(tr("ODE Solver Configuration"));
    QFont odeFont = odeLabel->font();
    odeFont.setItalic(true);
    odeFont.setUnderline(true);
    odeLabel->setFont(odeFont);
    runConfigLayoutLeft->setWidget(1, QFormLayout::SpanningRole, odeLabel);

    m_runEulerStepSpinBox = new QDoubleSpinBox; // value is set in readSettings()
    m_runEulerStepSpinBox->setDecimals(3);
    m_runEulerStepSpinBox->setSingleStep(.001);
    m_runEulerStepSpinBox->setRange(.001, 1.);
    m_runEulerStepSpinBox->setAccelerated(true); // values will spin up/down fast
    m_runEulerStepSpinBox->setToolTip(tr("Parameter \"h\" used in Euler Solver."));
    runConfigLayoutLeft->addRow(tr("Step Size:"), m_runEulerStepSpinBox);

    runConfigLayoutRight->addRow(QString(" "), new QWidget); // dummy

    m_runSolverSelectComboBox = new QComboBox;
    m_runSolverSelectComboBox->addItem(tr("Euler (direct)"),
                                       QVariant::fromValue<unsigned int>(AsymReg::Euler));
    m_runSolverSelectComboBox->addItem(tr("Midpoint (2nd order)"),
                                       QVariant::fromValue<unsigned int>(AsymReg::Midpoint));
    m_runSolverSelectComboBox->addItem(tr("Runge Kutta (4th order)"),
                                       QVariant::fromValue<unsigned int>(AsymReg::RungeKutta));
    runConfigLayoutRight->addRow(tr("Select Solver:"), m_runSolverSelectComboBox);

    m_runEulerIterationSpinBox = new QSpinBox; // value is set in readSettings()
    m_runEulerIterationSpinBox->setRange(0, 25); // 0: auto, 1-25: manual
    m_runEulerIterationSpinBox->setSpecialValueText(tr("auto")); // show "auto" when set to 0
    runConfigLayoutRight->addRow(tr("Max. iterations:"), m_runEulerIterationSpinBox);

    QHBoxLayout *runConfigLayout = new QHBoxLayout; // H = horizontal to place them left & right
    runConfigLayout->addLayout(runConfigLayoutLeft);
    runConfigLayout->addLayout(runConfigLayoutRight);

    QGroupBox *runConfigGroupBox = new QGroupBox;
    runConfigGroupBox->setLayout(runConfigLayout);

    /* data source table widget: */
    m_dataSourceTableWidget = new DataSourceTableWidget;
    m_dataSourceTableWidget->setRowCount(ASYMREG_DATSRC_SIZE);
    m_dataSourceTableWidget->setColumnCount(ASYMREG_DATSRC_SIZE);
    connect(m_dataSourceTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(dataSourceChanged(QTableWidgetItem*)));

    /* data source file actions: */
    QAction *emptyFileAction = new QAction(this);
    emptyFileAction->setText(tr(MW_DATSRC_STR_EMPTY));
    emptyFileAction->setCheckable(true);
    emptyFileAction->setChecked(true);
    emptyFileAction->setEnabled(false);

    QAction *clearListAction = new QAction(this);
    clearListAction->setText(tr(MW_DATSRC_STR_CLEAR));
    clearListAction->setToolTip(tr("Closes the selected data source and clears the list of possible files."));
    clearListAction->setIcon(QIcon::fromTheme("edit-clear"));

    QAction *selectFileAction = new QAction(this);
    selectFileAction->setText(tr(MW_DATSRC_STR_SELECT));
    selectFileAction->setToolTip(tr("Opens a filedialog to select a new source-file."));
    selectFileAction->setIcon(QIcon::fromTheme("document-open"));

    m_dataSourceSelectGroup = new QActionGroup(this);
    m_dataSourceSelectGroup->addAction(emptyFileAction);
    m_dataSourceSelectGroup->addAction(clearListAction);
    m_dataSourceSelectGroup->addAction(selectFileAction);
    connect(m_dataSourceSelectGroup, SIGNAL(selected(QAction*)),
            this, SLOT(selectDataSource(QAction*)));

    Menu *dataSourceSelectMenu = new Menu;
    dataSourceSelectMenu->addActions(m_dataSourceSelectGroup->actions());
    dataSourceSelectMenu->insertSeparator(clearListAction);

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

    /* data source plotting: */
    QPushButton *dataSourcePlotButton = new QPushButton;
    dataSourcePlotButton->setText(tr("Plot Data Source"));
    dataSourcePlotButton->setToolTip(tr("Plots the interpolated data from the current selected data source."));
    connect(dataSourcePlotButton, SIGNAL(clicked()),
            this, SLOT(plotDataSource()));

    QFileInfo plotConfigFile(MW_PLTCFG_FILE);
    QString plotConfigFileText = plotConfigFile.canonicalFilePath();

    QAction *currentPlotConfigAction = new QAction(this);
    currentPlotConfigAction->setText(plotConfigFile.exists() ?
                                         elidedFileName(plotConfigFileText) : tr("default"));
    currentPlotConfigAction->setData(plotConfigFileText);
    currentPlotConfigAction->setToolTip(plotConfigFileText);
    currentPlotConfigAction->setCheckable(true);
    currentPlotConfigAction->setChecked(true);
    currentPlotConfigAction->setEnabled(false);

    QAction *configurePlotConfigAction = new QAction(this);
    configurePlotConfigAction->setText(tr("Adjust Configuration"));
    configurePlotConfigAction->setToolTip(tr("Opens the configuration dialog to adjust your configuration."));
    configurePlotConfigAction->setIcon(QIcon::fromTheme("preferences-other"));

    m_plotConfigSelectGroup = new QActionGroup(this);
    m_plotConfigSelectGroup->addAction(currentPlotConfigAction);
    m_plotConfigSelectGroup->addAction(configurePlotConfigAction);
    connect(m_plotConfigSelectGroup, SIGNAL(selected(QAction*)),
            this, SLOT(selectPlotConfig(QAction*)));

    Menu *plotConfigSelectMenu = new Menu;
    plotConfigSelectMenu->addActions(m_plotConfigSelectGroup->actions());
    plotConfigSelectMenu->insertSeparator(configurePlotConfigAction);

    m_plotConfigSelectButton = new QPushButton;
    m_plotConfigSelectButton->setText(tr("Select Plotter Configuration"));
    m_plotConfigSelectButton->setToolTip(tr("Click here to select the configuration for data plotting."));
    m_plotConfigSelectButton->setMenu(plotConfigSelectMenu);

    /* buttons' layout: */
    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(m_dataSourceSelectButton);
    buttonLayout->addWidget(m_dataSourceSaveButton);
    buttonLayout->addWidget(dataSourcePlotButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(m_plotConfigSelectButton);
    buttonLayout->addStretch(2);

    /* Area with Scrollbar for all "main"/"middle" widgets: */
    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(m_dataSourceTableWidget);
    middleLayout->addLayout(buttonLayout);

    QWidget *middleWidget = new QWidget;
    middleWidget->setLayout(middleLayout);

    QScrollArea *middleArea = new QScrollArea;
    middleArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    middleArea->setWidget(middleWidget);
    middleArea->setWidgetResizable(true); // widgets will fill available space

    /* run asymreg: */
    m_runAsymRegButton = new QPushButton;
    m_runAsymRegButton->setCheckable(true); // button will stay pressed until regularization is finished
    m_runAsymRegButton->setText(tr("&Run Asymptotical Regularization"));
    m_runAsymRegButton->setToolTip(tr("Starts the mathematical part of this programm."));
    m_runAsymRegButton->setIcon(QIcon::fromTheme("media-playback-start"));

    connect(m_runAsymRegButton, SIGNAL(toggled(bool)), // disable middleArea Widget when button is checked
            middleArea, SLOT(setDisabled(bool)));
    connect(m_runAsymRegButton, SIGNAL(toggled(bool)), // disable runConfigGroupBox -"-
            runConfigGroupBox, SLOT(setDisabled(bool)));
    connect(m_runAsymRegButton, SIGNAL(clicked(bool)), // trigger runAsymReg() function
            this, SLOT(runAsymReg()));

    /* central widget with layout: */
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(infoLabel);
    layout->addWidget(runConfigGroupBox, 0);
    layout->addWidget(middleArea, 1);
    layout->addWidget(m_runAsymRegButton);

    setCentralWidget(new QWidget); // our Central-Widget is just an empty QWidget...
    centralWidget()->setLayout(layout); // ...which holds the main layout

    /* init file-system-watcher to recognize when dislin has finished: */
    QDir dir(QDir::currentPath());
    m_plotWatcher = new QFileSystemWatcher;
    m_plotWatcher->addPath(dir.canonicalPath());
    connect(m_plotWatcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(showSvgViewer(QString)));

    /* init matrix/vector: */
    zMat.resize(ASYMREG_DATSRC_SIZE, ASYMREG_DATSRC_SIZE);
    zMat.setZero();
    loadDataSourceToTableWidget();

    /* restore settings: */
    readSettings(); // read window-size, file-lists, etc

    /* load plotter configuration: */
    m_pressureFunctionPlotSettings = new ContourPlotterSettings;
    connect(m_pressureFunctionPlotSettings, SIGNAL(settingsChanged()),
            this, SLOT(changedPlotConfig()));
    selectPlotConfig(currentPlotConfigAction); // read preset file, TODO: Move to readSettings()

    /* make 'Enter' key hit the run-Button: */
    m_runAsymRegButton->setDefault(true);
    m_runAsymRegButton->setFocus();

    /* auto run regularization (this is more a debug option): */
    if (m_autoRunAction->isChecked()) {
        QMetaObject::invokeMethod(this, "runAsymReg",
                                  Qt::QueuedConnection);
    }
}

MainWindow::~MainWindow()
{
    /* delete class members: */
    delete m_dataSourceSelectButton->menu(); // found by valgrind
    delete m_plotConfigSelectButton->menu(); //   ^^ delete our Menu-classes, as they have no parent
    delete m_pressureFunctionPlotSettings;   // found by valgrind
    delete m_plotWatcher;

    /* clean up plot files: */
    QDir dir(QDir::currentPath());
    QStringList nameFilters = {"*.svg", "*.met"};
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
    newDataSourceAction->setText(fontMetrics().elidedText(fileName,
                                                          static_cast<Qt::TextElideMode>(FileNameElideMode),
                                                          FileNameElideSize));
    newDataSourceAction->setToolTip(fileName);
    newDataSourceAction->setData(fileName);
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
    QString text = fileName.isEmpty() ? tr("The data source has not been saved.")
                                      : tr("The data source \"%1\" has been modified.")
                                        .arg(fileInfo.fileName());
    QString infoText = fileName.isEmpty() ? tr("Please select a filename or discard your data.")
                                          : tr("Do you want to save or discard your changes?");

    // construct dialog:
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setInformativeText(infoText);
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setIcon(fileName.isEmpty() ? QMessageBox::Question : QMessageBox::Warning);
    if (fileName.isEmpty())
        msgBox.button(QMessageBox::Save)->setText(tr("&Save As")); // change default text to "Save As"
                                                                   // as we need a new file

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

void MainWindow::closeAllPlots()
{
    /* close all non-blocking dislin-xwin plotter: */
    Plotter::closeAllRemainingPlotter();

    /* also close all svg viewer */
    if (!m_svgViewerList.isEmpty()) {
        auto it = m_svgViewerList.begin();
        do {
            if (*it)
                (*it)->close();
        } while (++it != m_svgViewerList.end());
    }

    /* disable action again until another graf is plotted: */
    m_closeAllPlotsAction->setDisabled(true);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Plotter::closeAllRemainingPlotter(); // close all non-blocking dislin-xwin plotter
    m_closeAllPlotsAction->setDisabled(true); // disable qaction for now (look few lines below)

    if (isWindowModified()) {
        if (m_dataSourceChanged) {
            QAction *dataSource = m_dataSourceSelectGroup->checkedAction();
            int answer = askToSaveDataSource((dataSource != nullptr) ? dataSource->data().toString()
                                                                     : QString());
            if (answer == QMessageBox::Cancel) {
                event->ignore();
                return;
            }
        } else if (m_plotConfigChaned) {
            QAction *plotConfig = m_plotConfigSelectGroup->checkedAction();
            int answer = askToSavePlotConfig(plotConfig->data().toString());
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
                    m_closeAllPlotsAction->setDisabled(false); // okay we do need this qaction
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
    QVariant var = item->data(DataSourceTableWidgetItem::DataRole);
    Q_ASSERT(var.canConvert(QVariant::Double)); // DataSourceTableWidgetItem should take care of this,
                                                // so this is only for debugging the logic behind it

    /* set new (by user entered) value to zMat: */
    int col = m_dataSourceTableWidget->column(item);
    int row = m_dataSourceTableWidget->row(item);
    zMat(row, col) = var.toDouble();

    /* decorate window's title bar to show modification: */
    m_dataSourceChanged = true;                                   // make sure we know what has changed
    m_dataSourceSaveButton->setEnabled(m_dataSourceChanged);      // enable saving of data
    setWindowModified(m_dataSourceChanged || m_plotConfigChaned); // show asterik in widowtitle
}

void MainWindow::discardDataSource()
{
    if (!isWindowModified() || !m_dataSourceChanged)
        return;

    m_dataSourceChanged = false;
    setWindowModified(m_dataSourceChanged || m_plotConfigChaned);
    m_dataSourceSaveButton->setEnabled(m_dataSourceChanged);
}

QString MainWindow::elidedFileName(const QString &fileName) const
{
    QFontMetrics fm = fontMetrics();
    Qt::TextElideMode em = static_cast<Qt::TextElideMode>(FileNameElideMode);

    return fm.elidedText(fileName, em, FileNameElideSize);
}

void MainWindow::loadDataSourceToTableWidget()
{
    m_dataSourceTableWidget->blockSignals(true); // do not trigger 'itemChanged'-Signal

    for (int r = 0; r < zMat.rows(); ++r) {
        for (int c = 0; c < zMat.cols(); ++c) {
            QTableWidgetItem *item = m_dataSourceTableWidget->item(r, c);
            if (item == nullptr) {
                item = m_dataSourceTableWidget->itemPrototype()->clone();
                m_dataSourceTableWidget->setItem(r, c, item);
            }

            item->setData(DataSourceTableWidgetItem::DataRole, zMat.coeff(r,c));
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
    if (AsymReg::sourceFunction() == nullptr)
        return;

    Q_ASSERT(m_pressureFunctionPlotSettings != nullptr);

    Duration dur;
    auto Z = AsymReg::sourceFunctionPlotData(&dur);

    auto dt = dur.value();
    auto unit = dur.unit();
    statusBar()->showMessage(tr("Interpolation Time: %L1%2").arg(dt, 0, 'f', 3).arg(unit));

    ContourPlotter plotter(m_pressureFunctionPlotSettings, Plotter::Output_SVG_Image);
    plotter.setData(Z);

    m_plotImageTitleStack.push(tr("Transducer Pressure"));

    /* enable close-action: */
    m_closeAllPlotsAction->setEnabled(true);
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
        settings.beginGroup("Toolbar");
            m_autoPlotToolButton->setChecked(settings.value("autoPlot", true).toBool());
            m_autoPlotDataSrcAction->setChecked(settings.value("autoPlotDataSrc", true).toBool());
            m_autoPlotDataRegAction->setChecked(settings.value("autoPlotDataReg", true).toBool());
            m_autoRunAction->setChecked(settings.value("autoRun", false).toBool());
        settings.endGroup(); // "Toolbar"
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
        settings.beginGroup("AlgoRuntimeConfig");
            m_runGridSizeSpinBox->setValue(settings.value("grid-size", ASYMREG_GRID_SIZE).toInt());
            m_runSolverSelectComboBox->setCurrentIndex(settings.value("solver", 0).toInt());
            m_runEulerStepSpinBox->setValue(settings.value("euler-step", H).toDouble());
            m_runEulerIterationSpinBox->setValue(settings.value("euler-iter", T).toInt());
        settings.endGroup(); // "AlgoRuntimeConfig"
    settings.endGroup(); // "Main"

    if (dataSource != nullptr) {
        QMetaObject::invokeMethod(this, "selectDataSource",
                                  Qt::QueuedConnection,
                                  Q_ARG(QAction *, dataSource));
    }
}

void MainWindow::runAsymReg()
{
    bool autoPlot = m_autoPlotToolButton->isChecked();

    AsymReg::createSourceFunction(zMat);
    if (autoPlot && m_autoPlotDataSrcAction->isChecked())
        plotDataSource();

    AsymReg::generateDataSet();

    AsymReg::ODE_Solver solver = static_cast<AsymReg::ODE_Solver>(
                m_runSolverSelectComboBox->itemData(m_runSolverSelectComboBox->currentIndex())
                .value<unsigned int>());
    Duration dur;
    const Matrix<double, Dynamic, Dynamic> &X =
            AsymReg::regularize(solver,
                                m_runEulerIterationSpinBox->value(),
                                m_runEulerStepSpinBox->value(),
                                /*autoPlot ? m_pressureFunctionPlotSettings :*/ nullptr,
                                &dur);

    auto dt = dur.value();
    auto unit = dur.unit();
    statusBar()->showMessage(tr("Regularization Time: %L1%2").arg(dt, 0, 'f', 3).arg(unit));

    if (autoPlot && m_autoPlotDataRegAction->isChecked()) {
        QString t2, t3, t4;
        t2 = "regularized";
        t3 = "[" + m_runSolverSelectComboBox->currentText() + ": "
                + QString::number(m_runEulerIterationSpinBox->value()) + " iteration(s), step "
                + QString::number(m_runEulerStepSpinBox->value(), 'f', 3) + "]";
        t4 = "[ mean error ]";

        Q_ASSERT(m_pressureFunctionPlotSettings != nullptr);
        ContourPlotterSettings sett;
        copySettings(m_pressureFunctionPlotSettings, &sett);
        sett.setTitle(t2.toStdString(), 2);
        sett.setTitle(t3.toStdString(), 3);
        sett.setTitle(t4.toStdString(), 4);

        ContourPlotter plotter(&sett, Plotter::Output_SVG_Image);
        plotter.setData(X);

        m_plotImageTitleStack.push(tr("Regularized Transducer Pressure"));

        /* enable close-action: */
        m_closeAllPlotsAction->setEnabled(true);
    }

    m_runAsymRegButton->setChecked(false); // raise button so it can be pressed again
                                           // will also automatically enable widgets
}

void MainWindow::saveDataSource()
{
    QAction *act = m_dataSourceSelectGroup->checkedAction();
    if (act == nullptr) {
        askToSaveDataSource();
        return;
    }

    QFileInfo fileInfo(act->data().toString());
    saveDataSource(fileInfo.canonicalFilePath());
}

void MainWindow::saveDataSource(const QString &fileName, bool reload)
{
    std::fstream fs;
    fs.open(fileName.toAscii().data(), std::fstream::out | std::fstream::trunc);
    Q_ASSERT(fs.is_open());
    fs << zMat.format(EIGEN_IOFMT_CSV);
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
        settings.beginGroup("Toolbar");
            settings.setValue("autoPlot", m_autoPlotToolButton->isChecked());
            settings.setValue("autoPlotDataSrc", m_autoPlotDataSrcAction->isChecked());
            settings.setValue("autoPlotDataReg", m_autoPlotDataRegAction->isChecked());
            settings.setValue("autoRun", m_autoRunAction->isChecked());
        settings.endGroup(); // "Toolbar"
        settings.beginGroup("DataSource");
            // save datasource file actions:
            settings.beginWriteArray("source");
            QList<QAction *> list =  m_dataSourceSelectGroup->actions();
            auto it = list.constBegin();
            int arrayIdx = 0;
            do {
                if ((*it)->isCheckable() && ((*it)->text() != tr(MW_DATSRC_STR_EMPTY))) {
                    settings.setArrayIndex(arrayIdx++);
                    settings.setValue("file", (*it)->data().toString());
                    settings.setValue("selected", (*it)->isChecked());
                }
            } while (++it != list.constEnd());
            settings.endArray();
        settings.endGroup(); // "DataSource"
        settings.beginGroup("AlgoRuntimeConfig");
            settings.setValue("grid-size", m_runGridSizeSpinBox->value());
            settings.setValue("solver", m_runSolverSelectComboBox->currentIndex());
            settings.setValue("euler-step", m_runEulerStepSpinBox->value());
            settings.setValue("euler-iter", m_runEulerIterationSpinBox->value());
        settings.endGroup(); // "AlgoRuntimeConfig"
    settings.endGroup(); // "Main"
}

void MainWindow::selectDataSource(QAction *action)
{
    Q_ASSERT(action != nullptr);
    static QAction *lastSelectedAction = nullptr;

    // unsaved changes? ask user if and where to save:
    if (isWindowModified() && m_dataSourceChanged) {
        QString file;
        if (lastSelectedAction == nullptr)
            file = tr(MW_DATSRC_STR_EMPTY);
        else
            file = lastSelectedAction->data().toString();
        int answer = askToSaveDataSource(file);
        if (answer == QMessageBox::Cancel) {
            lastSelectedAction->setChecked(true);
            return;
        } else if (answer == QMessageBox::Discard) {
            m_dataSourceChanged = false;
            setWindowModified(m_dataSourceChanged || m_plotConfigChaned);
        }
    }

    // now select from which file to import data:
    QString fileName;
    if (action->isCheckable()) // this one has a file name inside!
        fileName = action->data().toString();
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

    // try to import data to zMat:
    if (!fileName.isEmpty()) {
        QString exceptionString;
        std::fstream fs;
        try {
            fs.open(fileName.toAscii().data(), std::fstream::in);
            fs.exceptions(std::ios_base::failbit | std::ios_base::badbit); // throw execptions on bad and fail
            fs >> zMat.format2(EIGEN_IOFMT_CSV);
            fs.close();
        } catch (const std::ios_base::failure &e) {
            exceptionString = tr("Caught an ios_base::failure.\n"
                             "Explanatory string: %1\n")
                    .arg(e.what());
        } catch (const IOFormatException &e) {
            exceptionString = e.what();
        } catch (...) {
            exceptionString = tr("Unknown Exception");
        }

        // did something fail?
        // (Exceptions are only for debugging)
        if (fs.fail() || !exceptionString.isEmpty()) {
            // prepare information:
            QFileInfo fileInfo(fileName);
            QString title = tr("Import Data Source - %1").arg(qApp->applicationName());
            QString text = tr("Failed to import data from file \"%1\"!").arg(fileInfo.fileName());

            // construct error dialog:
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(title);
            msgBox.setText(text);
            msgBox.setDetailedText(exceptionString);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();

            // loading failed so uncheck action:
            action->setChecked(false); // TODO: should we rather delete action???
            action = nullptr; // to preserve lastSelectedAction logic for saving data

            zMat.setZero(); // clear incomplete data
        }
    } else
        zMat.setZero(); // clear matrix

    // save QAction pointer for later use:
    lastSelectedAction = action;

    // update TableWidget:
    loadDataSourceToTableWidget();
}

void MainWindow::selectPlotConfig(QAction *action)
{
    Q_ASSERT(action != nullptr);
    Q_ASSERT(m_pressureFunctionPlotSettings != nullptr);

    if (action->isCheckable()) {
        QFileInfo fi(action->data().toString());
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

    // restore defaults may have deleted and new'ed the settings-pointer
    // also connect signals, if they are not already there
    // old signal-slot connection got auto-removed by destruction of
    // QObject (PlotterSettings is a QObject)
    m_pressureFunctionPlotSettings =
            dynamic_cast<ContourPlotterSettings *>(dialog.getPlotterSettings());
    connect(m_pressureFunctionPlotSettings, SIGNAL(settingsChanged()),
            this, SLOT(changedPlotConfig()), Qt::UniqueConnection);
}

void MainWindow::showSvgViewer(const QString &path)
{
    // use QDir class to filter all image files in 'path'
    // and sort by modification date:
    QStringList nameFilters = {"*.svg"}; // TODO: add more image types, eg "*.png", "*.jpg"
    QStringList images = QDir(path).entryList(nameFilters, QDir::Files, QDir::Time);

    // iterate over returned list to open all pictures
    // with newer mod.time than m_plotTime.
    auto it = images.constBegin();
    while (it != images.constEnd()) {
        if (QFileInfo(*it).lastModified() <= m_plotTime)
            break;

        // take last entry from title-stack:
        QString title = m_plotImageTitleStack.pop();
        if (title.isEmpty())
            title = tr("unknown image");

        // open svg-viwer with the latest image and the latest title:
        SvgViewer *svgViewer = new SvgViewer(*it++, title);
        m_svgViewerList << svgViewer;
        svgViewer->show();
    }

    // save 'now' for the next time we want to open image files:
    m_plotTime = QDateTime::currentDateTime();
}

#include "moc_mainwindow.cpp"
