#include "mainwindow.h"

#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>

#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QIcon>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QToolBar>
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

MainWindow::MainWindow()
    : m_pressureFunctionPlotSettings(nullptr),
      m_plotTime(QDateTime::currentDateTime())
{
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

    setCentralWidget(new QWidget);
    centralWidget()->setLayout(layout);

    QDir dir(QDir::currentPath());
    m_plotWatcher = new QFileSystemWatcher;
    m_plotWatcher->addPath(dir.canonicalPath());
    connect(m_plotWatcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(showSvgViewer(QString)));
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

void MainWindow::closeEvent(QCloseEvent *event)
{
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

    MatrixXd zMat;
    zMat.resize(11, 11);
    zMat.setZero();

    std::fstream fs;
    //fs.open("../data/data.csv", std::fstream::out | std::fstream::trunc);
    fs.open("../data/data.csv", std::fstream::in);
    fs >> zMat;
    fs.close();

    auto t1 = std::chrono::high_resolution_clock::now();
    func = new BilinearInterpol(xVec, yVec, zMat);
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
