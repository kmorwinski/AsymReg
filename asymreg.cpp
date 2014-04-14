#include "asymreg.h"

#include <cfloat>
#include <cmath>
#include <iostream>

#include "duration.h"
#include "interpol.h"

// namespaces:
using namespace Eigen;
using hrc = std::chrono::high_resolution_clock;

// init static members:
BilinearInterpol *AsymReg::m_sourceFunc(nullptr);

// function implementations:
void AsymReg::createSourceFunction(const MatrixXd &srcDat)
{
    VectorXd xVec = VectorXd::LinSpaced(ASYMREG_DATSRC_SIZE, 0., 10.);

    auto func = new BilinearInterpol(xVec, xVec, srcDat);
    AsymReg::setSourceFunction(func);
}

void AsymReg::generateDataSet(Duration *time)
{
    const int N = ASYMREG_RECORDING_ANGLES;

    auto t1 = hrc::now();
    ArrayXd phi = ArrayXd::LinSpaced(N, 0., M_PI - DBL_EPSILON);
    ArrayXXd sigmas(N, 2);
    sigmas.col(0) = phi.cos();
    sigmas.col(1) = phi.sin();
    std::cout << "Sigmas:\n" << sigmas << std::endl;
    auto t2 = hrc::now();

    if (time != nullptr)
        *time = t2 - t1;
}

MatrixXd AsymReg::sourceFunctionPlotData(Duration *time)
{
    assert(m_sourceFunc != nullptr);

    auto t1 = hrc::now();
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
    auto t2 = hrc::now();

    if (time != nullptr)
        *time = t2 - t1;

    return Z;
}

void AsymReg::setSourceFunction(BilinearInterpol *func)
{
    assert(func != nullptr);

    delete m_sourceFunc;
    m_sourceFunc = func;
}
