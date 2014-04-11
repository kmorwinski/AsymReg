#include "asymreg.h"

#include <chrono>
#include <iostream>

#include "interpol.h"

// namespaces:
using namespace Eigen;

// init static members:
BilinearInterpol *AsymReg::m_sourceFunc(nullptr);

// function implementations:
void AsymReg::createSourceFunction(const MatrixXd &srcDat)
{
    VectorXd xVec = VectorXd::LinSpaced(ASYMREG_DATSRC_SIZE, 0., 10.);

    auto func = new BilinearInterpol(xVec, xVec, srcDat);
    AsymReg::setSourceFunction(func);
}

MatrixXd AsymReg::sourceFunctionPlotData(double *time)
{
    assert(m_sourceFunc != nullptr);

    auto t1 = std::chrono::high_resolution_clock::now();
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
    auto t2 = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()/1000.;

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
