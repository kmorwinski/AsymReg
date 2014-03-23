#include "mainwindow.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

#include <discpp.h>

#include <chrono>
#include <iostream>
#include <fstream>

#include "eigen.h"
#include "interpol.h"

using namespace Eigen;

static double xray[101];
static double yray[101];
static double zheight[101][101];

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
    int n = 101;

    Dislin g;
    g.metafl("cons");
    g.scrmod("revers");
    g.disini();
    g.pagera();
    g.complx();
    //g.axspos(450, 1800);
    //g.axslen(2200, 1200);
    g.axspos(300, 1850);
    g.ax3len(2200, 1400, 1400);

    g.name("X-axis", "x");
    g.name("Y-axis", "y");
    g.name("Z-Achse", "z");

    g.labdig(-1, "x");
    g.ticks(9, "x");
    g.ticks(10, "y");

    g.titlin("Demonstration of BilinearInterpol", 1);

    g.graf3(0.0, 10.0, 0.0, 1.0, 0.0, 10.0, 0.0, 1.0, -2.0, 2.0, -2.0, 1.0);
    g.crvmat((double *)zheight, n, n, 1, 1);

#if 0
    g.titlin("Demonstration of LinearInterpol", 1);

    int ic = g.intrgb(0.95,0.95,0.95);
    g.axsbgd(ic);

    g.graf(0.0, 10.0, 1.0, 1.0, -5.0, 5.0, -5.0, 1.0);
    g.setrgb(0.7, 0.7, 0.7);
    g.grid(1, 1);

    g.color("fore");
    g.height(50);
    g.title();

    g.color("red");
    g.curve(xray, yray, n);
#endif // 0

    g.height(50);
    g.title();
    g.disfin();
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
    BilinearInterpol pressureFunction(xVec, yVec, zMat);


    int n = 101;
    double step = 10. / (n - 1);
    for (int i = 0; i < n; i++) {
        //xray[i] = i * step;
        //yray[i] = pressureFunction.interpol(xray[i]);
        double x = i * step;
        for (int j = 0; j < n; ++j) {
            double y = j * step;
            zheight[i][j] = pressureFunction.interpol(x, y);
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
    std::cout << "Bilieaner interpolation of pressure function took "
                  << dt << "µs" << std::endl;
}

#include "mainwindow.moc"
