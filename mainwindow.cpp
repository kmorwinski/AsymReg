#include "mainwindow.h"

#include <QtGui/QAction>
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
}

MainWindow::~MainWindow()
{}

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
    ContourPlotter plotter(*m_pressureFunctionPlotSettings, Plotter::Display_Widget);
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

#include "mainwindow.moc"
