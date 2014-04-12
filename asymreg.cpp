#include "asymreg.h"

#include <cfloat>
#include <chrono>
#include <cmath>
#include <iostream>

#include "interpol.h"

// namespaces:
using namespace Eigen;
using namespace std::chrono;

// init static members:
BilinearInterpol *AsymReg::m_sourceFunc(nullptr);

// function implementations:
void AsymReg::createSourceFunction(const MatrixXd &srcDat)
{
    VectorXd xVec = VectorXd::LinSpaced(ASYMREG_DATSRC_SIZE, 0., 10.);

    auto func = new BilinearInterpol(xVec, xVec, srcDat);
    AsymReg::setSourceFunction(func);
}

void AsymReg::generateDataSet(double *time)
{
    const int N = ASYMREG_RECORDING_ANGLES;

    ArrayXd phi = ArrayXd::LinSpaced(N, 0., M_PI - DBL_EPSILON);
    ArrayXXd sigmas(N, 2);
    sigmas.col(0) = phi.cos();
    sigmas.col(1) = phi.sin();
    std::cout << "Sigmas:\n" << sigmas << std::endl;
}

MatrixXd AsymReg::sourceFunctionPlotData(double *time)
{
    assert(m_sourceFunc != nullptr);

    auto t1 = high_resolution_clock::now();
    VectorXd X = VectorXd::LinSpaced(Sequential, ASYMREG_GRID_SIZE, 0., 10.);
    MatrixXd Z(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE);
    MatrixXd::Index i = 0;
    for (double &x : X) {
        auto j = 0;
        for (double &y : X) {
            Z(i,j) = m_sourceFunc->interpol(x,y);
            j++;
        }
        i++;
    }
    auto t2 = high_resolution_clock::now();
    auto dt = duration_cast<std::chrono::microseconds>(t2-t1).count()/1000.;

    if (time != nullptr)
        *time = dt;

    return Z;
}

void AsymReg::setSourceFunction(BilinearInterpol *func)
{
    assert(func != nullptr);

    delete m_sourceFunc;
    m_sourceFunc = func;
}
