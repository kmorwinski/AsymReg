#include "asymreg.h"

#include <cfloat>
#include <cmath>
#include <iostream>

#include "backprojection.h"
#include "constants.h"
#include "duration.h"
#include "eigen.h"
#include "interpol.h"
#include "plotter.h"
#include "plottersettings.h"
#include "radonoperator.h"

// defines:
#define STDOUT_MATRIX(MAT) \
    std::cout << #MAT" =" << std::endl \
              << MAT << std::endl << std::endl

// namespaces:
using hrc = std::chrono::high_resolution_clock;

/**
 * @brief Little helper class to transform coordinates from target
 *        to physical coord. system and get data from Asymreg::sourceFunction().
 */
class SrcFuncAccOp {
public:
    SrcFuncAccOp(BilinearInterpol *func)
        : m_func(func) {}

    /**
     * @brief Pass a 2D vector or 'list' of 2D vectors and get source-function data in return.
     * The given points @a pts are supposed to be in the target coord. system and are
     * therefore transformed as described below in the code.
     * After that the BilinearInterpol class is queried for every point and all values
     * are returned as a vector.
     * @param pts 2D Vector or Matrix/Array where each column represents a 2D Vector
     * @return Row-Vector with data values
     */
    template <typename Derived>
    Matrix<typename Derived::Scalar, 1, Dynamic> operator()(const EigenBase<Derived> &pts) const
    {
        typedef typename Derived::Index Index;
        typedef typename Derived::Scalar Scalar;

        EIGEN_STATIC_ASSERT(Derived::RowsAtCompileTime == 2,
                            THIS_METHOD_IS_ONLY_FOR_OBJECTS_OF_A_SPECIFIC_SIZE);

        /* tr:
         * ===
         * coordinate transformation from target coord. system (r,s) \in [0,1]^2
         * to physical coord. system (x,y) = [0,10]^2
         * We want: tr(r,s) = (x,y)|T = [2,8]^2  for r,s=0,...,1
         *  => tr(r,s) = (3*r+5, 3*s+5)
         *
         * Todo: documents want (r,s) = D^1  (unit disc)
         */
        static Transform<Scalar, 2, Affine> tr = Translation2d(5, 5) * Scaling(3.0);

        /* translate pts to phys. coord. system: */
        Matrix<Scalar, 2, Dynamic> xys = tr * pts;
        eigen_assert((xys.minCoeff() >= 0.0) && (xys.maxCoeff() <= 10.0));

        /* get data from source-function at (x,y):*/
        Matrix<Scalar, 1, Dynamic> ret(xys.cols()); // init with correct size, even if size is 1
        for (Index i = 0; i < xys.cols(); ++i)
            ret[i] = m_func->interpol(xys(0,i), xys(1,i));

        return ret;
    }

private:
    BilinearInterpol *m_func;
};

template<class Interpol>
class TrgtFuncAccOp
{
public:
    template <typename Derived>
    TrgtFuncAccOp(const EigenBase<Derived> &dataValues)
    {
        EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived); // assert if we call c'tor with a matrix

        typedef typename Derived::Scalar Scalar;

        Matrix<Scalar, 1, Dynamic> X =
                Matrix<Scalar, 1, Dynamic>::LinSpaced(Sequential, dataValues.size(),
                                                      -1, 1);

        m_interpol = new Interpol(X, dataValues);
    }

    ~TrgtFuncAccOp()
    { delete m_interpol; }

    template <typename Scalar>
    Scalar operator()(const Scalar s) const
    {
        /* check out of bound, SchlierenData is only valid in [-1,1]: */
        if ((s < -1.) || (s > 1.))
            return Scalar(0);

        /* get data at s:*/
        Scalar ret = m_interpol->interpol(s);

        return ret;
    }

private:
    Interpol *m_interpol;
};

// init static members:
BilinearInterpol *AsymReg::m_sourceFunc(nullptr);
RowVectorXd AsymReg::m_DataSet[AR_NUM_REC_ANGL];
Matrix<double, Dynamic, Dynamic> AsymReg::m_Result;

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

    /* printing list of rec. angles to stdout: */
    std::cout << "Using the following " << N << " recording angels:" << std::endl
              << (Phi*PHI/M_PI).format(IOFormat(2, 0, "", "°, ", "", "", "", "°")) // comma-separated & '°' after number
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
        //std::cout << std::endl << "phi_" << n << " = " << (Phi(n)*PHI/M_PI) << std::endl;

        Array<double, 1, numSamples> Integral; // temporary vector for integrated data

        /* Radon:
         * ======
         * Radon Operator
         * Create an Instance of our Radon-Operator to calculate Schlieren Data.
         * This Instance uses class SrcFuncAccOp as an interface for
         * translation & data acces to our BilinearInterpol-class.
         * Integration boundaries of r-Axis is set to [-1,1] by squareBound().
         */
        SrcFuncAccOp sfao(sourceFunction());
        RadonOperator<SrcFuncAccOp, void (double, double *, double *)>
                Radon(sfao, squareBound, Sigma.col(n));

        /* iterate over all entries in vector S: */
        for (Index j = 0; j < numSamples; ++j)
            Integral(j) = Radon(S(j)); // perform Radon-Transform for s_j

        /* finally square each entry and save as dataSet for angle phi_n: */
        m_DataSet[n] = Integral.square();

    }
    auto t2 = hrc::now(); // Stop timing

    /* printing generated data y_n to stdout: */
    std::cout << "Resulting in the following undisturbed (delta = 0) data:" << std::endl;
    for (Index n = 0; n < N; ++n) {
        std::cout << "y_" << n << "(s) =" << std::endl
                  << m_DataSet[n] << std::endl
                  << std::endl;
    }

    std::cout << std::endl;

    if (time != nullptr)
        *time = t2 - t1;
}

Matrix<double, Dynamic, Dynamic> &AsymReg::regularize(int iterations, double step, const PlotterSettings *pl, Duration *time)
{
    typedef typename MatrixXd::Index Index;
    typedef typename MatrixXd::Scalar Scalar;


    double h = (step > 0.) ? step : H;

    /* prepare plotter settings: */
    ContourPlotterSettings *sett = nullptr;
    if (pl != nullptr) {
        sett = new ContourPlotterSettings;
        copySettings(pl, sett);
        sett->setTitle("regularized data", 2);
    }

    std::cout << "Solving ODE with direct Euler method:" << std::endl
              << "  -> step size h = " << H << std::endl
              << "  -> initial value X0 = " << X0_C
              << " matrix of R^[" << ASYMREG_GRID_SIZE << "x" << ASYMREG_GRID_SIZE << "]"
              << std::endl;

    auto t1 = hrc::now(); // Start timing
    Matrix<double, Dynamic, Dynamic> Xdot(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE);
    Xdot.setZero();
    //STDOUT_MATRIX(Xdot);

    Matrix<double, Dynamic, Dynamic> Xn(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE);
    Matrix<double, Dynamic, Dynamic> Noise
            = Matrix<double, Dynamic, Dynamic>::Ones(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE);
            //= Matrix<double, Dynamic, Dynamic>::Random(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE);
    //STDOUT_MATRIX(Noise);

    //Xn = sourceFunctionPlotData()  + 0.1 * Noise; // this is easy!
    Xn = X0_C * Noise;                              // this one is hard!
    //STDOUT_MATRIX(Xn);

    Matrix<double, 1, Dynamic> Xsi = Matrix<double, 1, Dynamic>::LinSpaced(
                Sequential, ASYMREG_GRID_SIZE, 0., 10.);

    ArrayXd Phi = ArrayXd::LinSpaced(Sequential, N+1, 0., M_PI).head(N); // head(N) => take only first N entries
    MatrixXd Sigma(2, N);
    Sigma.row(0) = Phi.cos(); // [cos(phi_0), cos(phi_1), ... , cos(phi_n)]
    Sigma.row(1) = Phi.sin(); // [sin(phi_0), sin(phi_1), ... , sin(phi_n)]

    const Index numSamples = 2/AR_TRGT_SMPL_RATE + 1;
    RowVectorXd S = VectorXd::LinSpaced(Sequential, numSamples, -1., 1.);

    int run = 0;
    int max = (iterations > 0) ? iterations : T;
    do {
        std::string itrStr = "Euler iteration no " + std::to_string(run + 1)
                             + " of " + std::to_string(max);
        std::cout << itrStr << std::endl;

        for (int n = 0; n < Sigma.cols(); ++n) {
            BilinearInterpol biInterp(Xsi, Xsi, Xn);
            SrcFuncAccOp sfao(&biInterp);
            RadonOperator<SrcFuncAccOp, void (double, double *, double *)>
                    Radon(sfao, squareBound, Sigma.col(n));

            Matrix<double, 1, numSamples> RadonData; // temporary vector for radon data
            for (Index j = 0; j < S.cols(); ++j)
                RadonData[j] = Radon(S.coeffRef(j));

            Matrix<double, 1, numSamples> SchlierenData; // temporary vector for schlieren data
            SchlierenData = RadonData.cwiseProduct(RadonData);

            Matrix<double, 1, numSamples> Diff = m_DataSet[n] - SchlierenData; // Diff = Y_delta - F(Xn)
            Matrix<double, 1, numSamples> DiffTimesRadon = RadonData.cwiseProduct(Diff); // DiffTimesRadon = R(Xn) * Diff
            //STDOUT_MATRIX(DiffTimesRadon);

            TrgtFuncAccOp<LinearInterpol> tfao(DiffTimesRadon);
            Backprojection<TrgtFuncAccOp<LinearInterpol> > R_adjoint(tfao, Sigma.col(n));

            Matrix<double, Dynamic, Dynamic> Xn_1(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE); // temporary matrix for backprojected data
            for (int k = 0; k < ASYMREG_GRID_SIZE; ++k) {
                for (int l = 0; l < ASYMREG_GRID_SIZE; ++l) {
                    Matrix<double, 2, 1> vec;
                    vec << Xsi(k), Xsi(l);

                    /* trInv:
                     * ======
                     * Inverse coordinate transformation from target coord. system
                     * (r,s) \in [0,1]^2 to physical coord. system (x,y) = [0,10]^2
                     * We want: tr(r,s) = (x,y)|T = [2,8]^2  for r,s=0,...,1
                     *  => tr(r,s) = (3*r+5, 3*s+5)
                     *  => trInv(x,y) = tr.inverse()(x,y) = (0.333*[x-5], 0.333*[y-5])
                     *
                     * Todo: documents want (r,s) = D^1  (unit disc)
                     */
                    static Transform<Scalar, 2, Affine> trInv = (Translation2d(5, 5) * Scaling(3.0)).inverse();
                    //STDOUT_MATRIX(trInv);

                    /* translate s to target coord. system: */
                    vec = trInv * vec;

                    /* backproject vector: */
                    Xn_1(k,l) = R_adjoint(vec);
                }
            }
            //STDOUT_MATRIX(Xn_1);

            Xdot += 1./double(N) * (Xn + 2*h*Xn_1);
            Xn = Xdot; // use regularized data for next iteration step
        }

        /* plot lastest iteration result: */
        if (sett != nullptr) {
            sett->setTitle(itrStr, 3);
            ContourPlotter plotter(sett, Plotter::Output_Display_Widget);
            plotter.setData(Xdot);
            plotter.plot(true); // keep open and do not block
        }

        //STDOUT_MATRIX(Xdot);
    } while (++run < max);
    auto t2 = hrc::now(); // Stop timing

    /* calculate and pass back time used: */
    if (time != nullptr)
        *time = t2 - t1;

    std::cout << std::endl; // put another linebreak to stdout for easier reading

    /* delete plottersettings copy: */
    delete sett;

    /* Xdot = Xn + 2hR*{R(Xn)[Y - F(Xn)]} */
    m_Result = Xdot;
    return m_Result;
}

Matrix<double, Dynamic, Dynamic> AsymReg::sourceFunctionPlotData(Duration *time)
{
    assert(m_sourceFunc != nullptr);

    auto t1 = hrc::now();
    VectorXd X = VectorXd::LinSpaced(Sequential, ASYMREG_GRID_SIZE, 0., 10.);
    MatrixXd Z(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE);

    for (int i = 0; i < X.rows(); ++i) {
        for (int j = 0; j < X.rows(); ++j)
            Z(i,j) = m_sourceFunc->interpol(X(i),X(j));
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
