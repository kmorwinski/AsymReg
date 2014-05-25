#include "asymreg.h"

#include <cfloat>
#include <cmath>
#include <iostream>

#include "eigen.h"
#include "duration.h"
#include "interpol.h"
#include "radonoperator.h"

// namespaces:
using hrc = std::chrono::high_resolution_clock;

class SrcFuncAccOp {
public:
    SrcFuncAccOp(BilinearInterpol *func)
        : m_func(func) {}

    double operator()(const Matrix<double, 2, 1> &pt)
    {
        /* tr:
         * ===
         * coordinate transformation from target coord. system (r,s) \in [0,1]^2
         * to physical coord. system (x,y) = [0,10]^2
         * We want: tr(r,s) = (x,y)|T = [2,8]^2  for r,s=0,...,1
         *  => tr(r,s) = (3*r+5, 3*s+5)
         *
         * Todo: documents want (r,s) = D^1  (unit disc)
         */
        static Transform<double, 2, Affine> tr = Translation2d(5, 5) * Scaling(3.0);

        /* translate pt to phys. coord. system: */
        Matrix<double, 2, 1> xy = tr * pt;

        /* get data from source-function at (x,y):*/
        double ret = m_func->interpol(xy.coeffRef(0), xy.coeffRef(1));

        return ret;
    }

private:
    BilinearInterpol *m_func;
};

// init static members:
BilinearInterpol *AsymReg::m_sourceFunc(nullptr);
RowVectorXd AsymReg::m_dataSet[AR_NUM_REC_ANGL];

// function implementations:
/**
 * @brief Returns [-1,1] as integration boundaries for Radon-Transform.
 * @param in (unused)
 * @param lower Pointer to lower boundary variable.
 * @param upper Pointer to upper boundary variable.
 */
void squareBound(double /*in*/, double *lower, double *upper)
{
    *lower = -1.;
    *upper = 1.;
}

void AsymReg::createSourceFunction(const MatrixXd &srcDat)
{
    VectorXd xVec = VectorXd::LinSpaced(ASYMREG_DATSRC_SIZE, 0., 10.);

    auto func = new BilinearInterpol(xVec, xVec, srcDat);
    AsymReg::setSourceFunction(func);
}

void AsymReg::generateDataSet(Duration *time)
{
    const int N = AR_NUM_REC_ANGL;
    typedef typename MatrixXd::Index Index;

    /* Phi:
     * ====
     * Recording angles in [0,pi] in N+1 equidistant steps,
     * but we skip the last (N+1)th step because it is the same as 0°.
     * We could use intervall [0,pi) but we will then never get the exact 90°
     * angle which means data sampling in direction of light-rays/xray.
     */
    static_assert(N % 2 == 0, "only even numbers of N are allowed"); // we want that 90° degree angle
    ArrayXd Phi = ArrayXd::LinSpaced(Sequential, N+1, 0., M_PI).head(N); // head(N) => take only first N entries

    std::cout << "Using the following " << N << " recording angels:" << std::endl
              << (Phi*180.0/M_PI).format(IOFormat(2, 0, "", "°, ", "", "", "", "°")) // comma-separated & '°' after number
              << std::endl << std::endl;

    auto t1 = hrc::now(); // Start timing (TODO: start before Phi, but exlude cout)
    /* Sigma:
     * ======
     * Recording angles Phi in euclidian coord. system.
     * Each column in Sigma represents one recording angle as target coords.
     */
    MatrixXd Sigma(2, N);
    Sigma.row(0) = Phi.cos(); // [cos(phi_0), cos(phi_1), ... , cos(phi_n)]
    Sigma.row(1) = Phi.sin(); // [sin(phi_0), sin(phi_1), ... , sin(phi_n)]

    const Index numSamples = 2/AR_TRGT_SMPL_RATE + 1;
    static_assert(numSamples % 2, "only values for AR_TRGT_SMPL_RATE with uneven"
                                  "result of \"(2/AR_TRGT_SMPL_RATE) + 1\" are allowed");

    /* S:
     * ==
     * Discrete samples of s-axis (target coord. system)
     * The basis for this sampling is the value defined in AR_TRGT_SMPL_RATE
     * S_i = -1 + i * numsamples, for i=0,1,2,... , and
     * where numsamples = (2/AR_TRGT_SMPL_RATE) + 1
     *
     * TODO: use grid size (like Plotter) as calculation basis
     */
    RowVectorXd S = VectorXd::LinSpaced(Sequential, numSamples, -1., 1.);

    //std::cout << "S =" << std::endl << S << std::endl << std::endl;

    /* iterate over all rec. angles: */
    for (Index n = 0; n < N; ++n) {
        //std::cout << std::endl << "phi_" << n << " = " << (Phi(n)*180.0/M_PI) << std::endl;

        Array<double, 1, numSamples> Integral; // temporary vector for integrated data

        /* Radon:
         * ======
         * Radon Operator
         * Create an Instance of our Radon-Operator to calculate Schlieren Data.
         * This Instance uses class SrcFuncAccOp as an interface for
         * translation & data Acces to our BilinearInterpol-class.
         * Integration boundaries of r-Axis is set to [-1,1].
         */
        SrcFuncAccOp sfao(sourceFunction());
        RadonOperator<SrcFuncAccOp, void (double, double *, double *)>
                Radon(sfao, squareBound, Sigma.col(n));

        /* iterate over all entries in vector S: */
        for (Index j = 0; j < numSamples; ++j)
            Integral(j) = Radon(S(j)); // perform Radon-Transform for s_j

        /* finally square each entry and save as dataSet for angle phi_n: */
        m_dataSet[n] = Integral.square();

    }
    auto t2 = hrc::now(); // Stop timing

    std::cout << "yDelta_n(s) = " << std::endl;
    for (Index n = 0; n < N; ++n)
        std::cout << n << ": " << m_dataSet[n] << std::endl;

    std::cout << std::endl;

    if (time != nullptr)
        *time = t2 - t1;
}

Matrix<double, Dynamic, Dynamic> AsymReg::sourceFunctionPlotData(Duration *time)
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
