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

using namespace std;
using namespace Eigen;

static double xray[101];
static double yray[101];
static double zheight[101][101];

static BilinearInterpol *func = nullptr;

MainWindow::MainWindow()
{
    QAction* a = new QAction(this);
    a->setText( "Quit" );
    connect(a, SIGNAL(triggered()), SLOT(close()) );
    menuBar()->addMenu( "File" )->addAction( a );

    QPushButton *runAsymRegButton = new QPushButton;
    runAsymRegButton->setText("run math");
    connect(runAsymRegButton, SIGNAL(clicked(bool)),
            this, SLOT(runAsymReg()));

    QPushButton *plotPressureFunctionButton = new QPushButton;
    plotPressureFunctionButton->setText("Plot Pressure Funtion");
    connect(plotPressureFunctionButton, SIGNAL(clicked(bool)),
        this, SLOT(plotPressureFunction()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(runAsymRegButton);
    layout->addWidget(plotPressureFunctionButton);

    setCentralWidget(new QWidget);
    centralWidget()->setLayout(layout);
}

MainWindow::~MainWindow()
{}

void MainWindow::plotPressureFunction()
{
    if (func == nullptr)
        return;

    Plotter::Span xSpan = std::make_tuple(0., 10.);
    Plotter::Span ySpan = make_tuple(0., 10.);
    Plotter::Span zSpan = make_tuple(-2., 2.);
    int steps = 100;

    double stepSize = abs(0. - 10.) / steps;
    float zmat[steps][steps];

    for (int i = 0; i < steps; i++) {
        double x = i * stepSize;
        for (int j = 0; j < steps; ++j) {
            double y = j * stepSize;
            zmat[i][j] = func->interpol(x, y);
        }
    }

    ContourPlotter plotter(.35, Plotter::Display_Widget);
    plotter.plotData(&zmat[0][0], xSpan, ySpan, zSpan, steps);
}

void MainWindow::runAsymReg()
{
    VectorXd xVec;
    xVec.setLinSpaced(11, 0.0, 10.0);
    VectorXd yVec(xVec);

    /*VectorXd yVec;
    yVec.resizeLike(xVec);
    yVec.setZero();
    yVec(1) = 2;
    yVec(2) = 2;
    yVec(3) = 3;
    yVec(4) = 3.5;
    yVec(5) = -1;
    yVec(6) = -5;

    LinearInterpol pressureFunction(xVec, yVec);*/
    //SplineInterpol pressureFunction(xVec, yVec);

    MatrixXd zMat;
    zMat.resize(xVec.size(), yVec.size());
    zMat.setZero();

    std::fstream fs;
    //fs.open("data.txt", std::fstream::out | std::fstream::trunc);
    fs.open("data.txt", std::fstream::in);
    fs >> zMat;
    fs.close();

    auto t1 = std::chrono::high_resolution_clock::now();
    func = new BilinearInterpol(xVec, yVec, zMat);
}

#include "mainwindow.moc"
