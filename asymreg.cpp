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
    typedef typename MatrixXd::Index Index;

    auto t1 = hrc::now();
    ArrayXd phi = ArrayXd::LinSpaced(N, 0., M_PI);// - DBL_EPSILON);
    MatrixXd sigmas(2, N);
    sigmas.row(0) = phi.cos();
    sigmas.row(1) = phi.sin();

    Rotation2Dd rot(M_PI/2); // 90° counter-clockwise shift

    std::cout << "Using " << N << " recording angles:" << std::endl;
    std::cout << sigmas << std::endl;

    const Index size = 1 + 2/AR_SAMP;
    static_assert(size % 2, "should be uneven to catch origin!!!");
    RowVectorXd r = VectorXd::LinSpaced(Sequential, size, -1., 1.);
    RowVectorXd s = r;

    Matrix<double, 2, size> lines[size];
    Matrix<double, 2, size> linesTR[size];
    Transform<double, 2, Affine> tr = Translation2d(5, 5) * Scaling(3.0);

    for (Index i = 0; i < N; ++i) {
        std::cout << std::endl << "phi_" << i << " = " << (phi(i)*180.0/M_PI) << std::endl;
        MatrixXd line1 = rot * sigmas.col(i) * r;

        for (Index j = 0; j < size; ++j) {
            if (s(j) == 0.)
                lines[j] = line1;
            else {
                lines[j].row(0) = line1.row(0).array() + s(j) * sigmas(0,i);
                lines[j].row(1) = line1.row(1).array() + s(j) * sigmas(1,i);
            }

            linesTR[j] = tr*lines[j];

            std::cout << "s_" <<  j << " = " << s(j) << std::endl;
            std::cout << lines[j] << std::endl;
            std::cout << "translated to physical coords: " << std::endl;
            std::cout << linesTR[j] << std::endl;
            std::cout << std::endl;
        }
    }
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
        MatrixXd::Index j = 0;
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
