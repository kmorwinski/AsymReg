#include "mainwindow.h"

#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>

#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

#include <chrono>
#include <iostream>
#include <fstream>

#include "eigen.h"
#include "interpol.h"
#include "plotter.h"
#include "plottersettingsdialog.h"
#include "svgviewer.h"

using namespace std;
using namespace Eigen;

static double xray[101];
static double yray[101];
static double zheight[101][101];

static BilinearInterpol *func = nullptr;

MainWindow::MainWindow()
    : m_pressureFunctionPlotSettings(nullptr)
{
    QAction* a = new QAction(this);
    a->setText(tr("Quit"));
    connect(a, SIGNAL(triggered()), SLOT(close()) );
    menuBar()->addMenu(tr("File"))->addAction( a );

    QPushButton *runAsymRegButton = new QPushButton;
    runAsymRegButton->setText(tr("Run Math"));
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
        ContourPlotterSettings *sett = new ContourPlotterSettings(*m_pressureFunctionPlotSettings);
        PlotterSettingsDialog dialog(this);
        dialog.setPlotterSettings(sett);
        auto ret = dialog.exec();
        if (ret == QDialog::Rejected)
            return;

        delete m_pressureFunctionPlotSettings;
        m_pressureFunctionPlotSettings = sett;
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

    m_pressureFunctionPlotSettings->setTitle("hier koennte ihr Titel stehen", 3);
    m_pressureFunctionPlotSettings->setAxisSpan(PlotterSettings::Span(0., 10.), PlotterSettings::X_Axis);
    m_pressureFunctionPlotSettings->setAxisSpan(PlotterSettings::Span(0., 10.), PlotterSettings::Y_Axis);
    m_pressureFunctionPlotSettings->setAxisSpan(PlotterSettings::Span(-3., 3.), PlotterSettings::Z_Axis);
    ContourPlotter plotter(*m_pressureFunctionPlotSettings, Plotter::SVG_Image);
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

void MainWindow::showSvgViewer(const QString &path)
{
    // use QDir class to filter all image files in 'path'
    // and sort by modification date:
    QStringList nameFilters = {"*.svg", "*.png", "*.jpg", "*.jpeg"}; // TODO: add more image types
    QStringList images = QDir(path).entryList(nameFilters, QDir::Files, QDir::Time);

    // assume top entry in images-list is the one that was output by dislin:
    SvgViewer *svgViewer = new SvgViewer("../data/err2.svg"/*images.at(0)*/, "a simple image");
    m_svgViewerList << svgViewer;
    svgViewer->show();
}

#include "mainwindow.moc"
