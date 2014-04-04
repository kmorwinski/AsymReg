#include "mainwindow.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>
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

using namespace std;
using namespace Eigen;

static BilinearInterpol *func = nullptr;

MatrixXd zMat;

static bool qaction_lessThan(QAction *ac1, QAction *ac2)
{
    return (ac1->text() <= ac2->text());
}

MainWindow::MainWindow()
    : m_pressureFunctionPlotSettings(nullptr),
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
    emptyFile->setText(tr("Empty"));
    emptyFile->setCheckable(true);
    emptyFile->setChecked(true);
    emptyFile->setEnabled(false);

    QAction *selectFile = new QAction(this);
    selectFile->setText(tr("Select File"));
    selectFile->setToolTip(tr("Opens a filedialog to select a new source-file."));
    selectFile->setIcon(QIcon::fromTheme("document-open"));

    m_dataSourceSelectGroup = new QActionGroup(this);
    m_dataSourceSelectGroup->addAction(emptyFile);
    m_dataSourceSelectGroup->addAction(selectFile);
    connect(m_dataSourceSelectGroup, SIGNAL(selected(QAction*)),
            this, SLOT(selectDataSource(QAction*)));

    QMenu *dataSourceSelectMenu = new QMenu;
    dataSourceSelectMenu->addActions(m_dataSourceSelectGroup->actions());
    dataSourceSelectMenu->insertSeparator(selectFile);

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

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(infoLabel, 0, 0, 1, 3);
    layout->addWidget(m_dataSourceTableWidget, 1, 0, 5, 2);
    layout->addWidget(m_dataSourceSelectButton, 1, 2, 1, 1);
    layout->addWidget(m_dataSourceSaveButton, 2, 2, 1, 1);

#if 0
    QPushButton *runAsymRegButton = new QPushButton;
    runAsymRegButton->setText(tr("Run Math"));
    runAsymRegButton->setDefault(true);
    connect(runAsymRegButton, SIGNAL(clicked(bool)),
            this, SLOT(runAsymReg()));

    m_confNplotPressFuncAction = new QAction(this);
    m_confNplotPressFuncAction->setText(tr("Configure And Plot Pressure Function"));

    m_plotPressFuncAction = new QAction(this);
    m_plotPressFuncAction->setText(tr("Plot Pressure Function"));

    QActionGroup *actionGroup = new QActionGroup(this);
    actionGroup->addAction(m_plotPressFuncAction);
    actionGroup->addAction(m_confNplotPressFuncAction);
    connect(actionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(plotPressureFunction(QAction *)));

    QMenu *buttonMenu = new QMenu;
    buttonMenu->addActions(actionGroup->actions());

    QPushButton *plotPressureFunctionButton = new QPushButton;
    plotPressureFunctionButton->setText(tr("Plot Options"));
    plotPressureFunctionButton->setMenu(buttonMenu);
    connect(plotPressureFunctionButton, SIGNAL(clicked(bool)),
        this, SLOT(plotPressureFunction(QAction *)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(runAsymRegButton);
    layout->addWidget(plotPressureFunctionButton);
#endif //0

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

QAction *MainWindow::addDataSourceAction(const QString &fileName)
{
    // remove "empty" action:
    QList<QAction *> list = m_dataSourceSelectGroup->actions();
    if ((list.size() == 2) && (list.at(0)->text() == tr("Empty"))) {
        QAction *empty = list.takeAt(0);
        delete empty;
    }

    // create new QAction:
    QAction *newDataSourceAction = new QAction(this);
    newDataSourceAction->setText(fileName);
    newDataSourceAction->setCheckable(true);
    newDataSourceAction->setActionGroup(m_dataSourceSelectGroup);
    newDataSourceAction->setChecked(true);

    // sort list to make sure data files come first (start with '/'):
    list << newDataSourceAction;
    qSort(list.begin(), list.end(), qaction_lessThan);

    // remove and re-add all actions:
    QMenu *menu = m_dataSourceSelectButton->menu();
    menu->clear(); // deletes separator, but keeps select-file-action
    menu->addActions(list);
    menu->insertSeparator(list.at(list.size()-1));

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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()) {
        QAction *dataSource = m_dataSourceSelectGroup->checkedAction();
        int answer = askToSaveDataSource(dataSource->text());
        if (answer == QMessageBox::Cancel) {
            event->ignore();
            return;
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
    setWindowModified(true); // show asterik in widowtitle
    m_dataSourceChanged = true; // make sure we know what has changed
    m_dataSourceSaveButton->setEnabled(true);

    m_dataSourceTableWidget->blockSignals(false);
}

void MainWindow::discardDataSource()
{
    if (!isWindowModified() || !m_dataSourceChanged)
        return;

    m_dataSourceChanged = false;
    setWindowModified(m_dataSourceChanged);
    m_dataSourceSaveButton->setEnabled(false);
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

bool MainWindow::loadPlotterSettings(const QString &fileName, PlotterSettings *sett) const
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

void MainWindow::plotPressureFunction(QAction *action)
{
    if (func == nullptr)
        return;

    bool needConfigDialog = false;

    if (m_pressureFunctionPlotSettings == nullptr) {
        needConfigDialog = true;
        m_pressureFunctionPlotSettings = new ContourPlotterSettings;
    }

    if (action == m_confNplotPressFuncAction)
        needConfigDialog = true;

    if (needConfigDialog) {
        PlotterSettingsDialog dialog(this);
        dialog.setPlotterSettings(m_pressureFunctionPlotSettings);
        auto ret = dialog.exec();
        if (ret == QDialog::Rejected)
            return;

        m_pressureFunctionPlotSettings =
                dynamic_cast<ContourPlotterSettings *>(dialog.getPlotterSettings());
    }

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

bool MainWindow::savePlotterSettings(const QString &fileName, const PlotterSettings *sett) const
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

void MainWindow::selectDataSource(QAction *action)
{
    Q_ASSERT(action != nullptr);
    static QAction *lastSelectedAction = nullptr;

    if (isWindowModified() && m_dataSourceChanged) {
        QString file;
        if (lastSelectedAction == nullptr)
            file = tr("Empty");
        else
            file = lastSelectedAction->text();
        int answer = askToSaveDataSource(file);
        if (answer == QMessageBox::Cancel) {
            lastSelectedAction->setChecked(true);
            return;
        } else if (answer == QMessageBox::Discard) {
            setWindowModified(false);
            m_dataSourceChanged = false;
        }
    }

    QString fileName;
    if (action->isCheckable())
        fileName = action->text();
    else { // Select Data Source
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
    }

    // save for later use:
    lastSelectedAction = action;

    // read data to zMat
    std::fstream fs;
    fs.open(fileName.toAscii().data(), std::fstream::in);
    Q_ASSERT(fs.is_open());
    fs >> zMat;
    fs.close();

    // update TableWidget:
    loadDataSourceToTableWidget();
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
