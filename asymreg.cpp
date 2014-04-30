#include "asymreg.h"

#include <cfloat>
#include <cmath>
#include <iostream>

#include "eigen.h"
#include "duration.h"
#include "interpol.h"

// namespaces:
using hrc = std::chrono::high_resolution_clock;

// init static members:
BilinearInterpol *AsymReg::m_sourceFunc(nullptr);
RowVectorXd AsymReg::m_dataSet[AR_NUM_REC_ANGL];

// function implementations:
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

    /* rot90:
     * ======
     * Rotation of 90° to get perpendicular angles.
     * Rotation2D class creates rot. matrix internally
     * Todo: matrix is created on every "rot90 * ..." operation.
     *       save rot-Matrix or create own one
     */
    Rotation2Dd rot90(M_PI_2); // M_PI/2

    const Index numSamples = 2/AR_TRGT_SMPL_RATE + 1;
    static_assert(numSamples % 2, "only values for AR_TRGT_SMPL_RATE with uneven result of \"2/AR_TRGT_SMPL_RATE + 1\" are allowed");

    /* R:
     * ==
     * Discrete samples of r-axis (target coord. system)
     * The basis for this sampling is the value defined in AR_TRGT_SMPL_RATE
     * R_i = -1 + i * numsamples, where numsamples = (2/AR_TRGT_SMPL_RATE) + 1
     *
     * TODO: use grid size (like Plotter) as calculation basis
     */
    RowVectorXd R = VectorXd::LinSpaced(Sequential, numSamples, -1., 1.);

    /* S:
     * ==
     * Discrete samples of s-axis (target coord. system)
     * (see above)
     */
    const RowVectorXd &S = R; // simply a ref to R because we use the same AR_SAMP

    //std::cout << "R =" << std::endl << R << std::endl << std::endl;
    //std::cout << "S =" << std::endl << S << std::endl << std::endl;

    Matrix<double, 2, numSamples> IntLines[numSamples];
    Matrix<double, 1, numSamples> IntData[N];

    /* tr:
     * ===
     * coordinate transformation from target coord. system (r,s) \in [0,1]^2
     * to physical coord. system (x,y) = [0,10]^2
     * We want: tr(r,s) = (x,y)|T = [2,8]^2  for r,s=0,...,1
     *  => tr(r,s) = (3*r+5, 3*s+5)
     *
     * Todo: documents want (r,s) = D^1  (unit disc)
     */
    Transform<double, 2, Affine> tr = Translation2d(5, 5) * Scaling(3.0);

    /* Trapez:
     * =======
     * Vector containing the coefficients for the extended trapezoidal rule.
     * Trapez = [1/2*numSamples, 1/numSamples, ..., 1/numSamples, 1/2*numSamples]
     */
    Matrix<double, numSamples, 1> Trapez;
    Trapez.setConstant(1./numSamples);
    Trapez(0) *= .5;
    Trapez(numSamples-1) *= .5;

    /* iterate over all rec. angles: */
    for (Index n = 0; n < N; ++n) {
        //std::cout << std::endl << "phi_" << n << " = " << (Phi(n)*180.0/M_PI) << std::endl;

        Array<double, 1, numSamples> Integral; // temporary vector for integrated data

        /* LineOrigin:
         * ===========
         * Standard line which crosses the origin: r*sigma^{\perp} for r=-1,...,1
         */
        Matrix<double, 2, numSamples> LineOrigin = rot90 * Sigma.col(n) * R;

        /* translate LineOrigin along projection of
         * s * sigma_n, for s=-1,...,1
         */
        for (Index j = 0; j < numSamples; ++j) {
            IntLines[j] = LineOrigin;

            if (S(j) != 0.)
                IntLines[j].colwise() += S(j) * Sigma.col(n);

            //std::cout << "s_" <<  j << " = " << S(j) << std::endl
            //          << IntLines[j] << std::endl;

            /* translate calculated lines to phys. coord. system: */
            IntLines[j] = tr * IntLines[j];

            //std::cout << "translated to physical coords: " << std::endl
            //          << IntLines[j] << std::endl << std::endl;

            /* get data from source-function: */
            for (int k = 0; k < IntLines[j].cols(); ++k)
                IntData[n](0,k) = sourceFunction()->interpol(IntLines[j](0,k), IntLines[j](1,k));

            //std::cout << "data to be integrated = " << std::endl
            //          << IntData[n] << std::endl << std::endl;

            /* calculate trapezoidal: */
            Integral(j) = IntData[n] * Trapez;

            //std::cout << "integral =" << std::endl
            //          << m_dataSet[n](j) << std::endl << std::endl;
        }

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
