#include "asymreg.h"

#include <cfloat>
#include <cmath>
#include <iostream>

#include "backprojection.h"
#include "constants.h"
#include "duration.h"
#include "eigen.h"
#include "interpol.h"
#include "ode.h"
#include "plotter.h"
#include "plottersettings.h"
#include "radonoperator.h"

// defines:
#define STDOUT_MATRIX(MAT) \
    std::cout << #MAT" =" << std::endl \
              << MAT << std::endl << std::endl

// namespaces:
using hrc = std::chrono::high_resolution_clock;

// local/private functions:
static void circleBound(double in, double *lower, double *upper);
static void squareBound(double in, double *lower, double *upper);

// template classes:
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
        static Transform<Scalar, 2, Affine> tr = Translation2d(5, 5) * Scaling(5.0);

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

/**
 * @brief Class which holds the right-hand-side of our regularization formula.
 */
template <typename DerivedMatrix, typename DerivedVector>
class DerivateOperator
{
    typedef typename DerivedVector::Scalar Scalar;

public:
    DerivateOperator(const EigenBase<DerivedMatrix> &Sigma,
                     const EigenBase<DerivedVector> &S, const EigenBase<DerivedVector> &Xsi,
                     const DerivedVector *DataSets)
        : m_Sigma(Sigma.derived()),
          m_S(S.derived()),
          m_Xsi(Xsi.derived()),
          m_DataSet(DataSets)
    {
        EIGEN_STATIC_ASSERT_VECTOR_ONLY(DerivedVector);
        assert(DataSets != nullptr);
    }

    template <typename Derived, typename OtherDerived>
    void error(const EigenBase<Derived> &X, EigenBase<OtherDerived> &Error)
    {
        EIGEN_STATIC_ASSERT_VECTOR_ONLY(OtherDerived);

        constexpr int numSamples = 2/AR_TRGT_SMPL_RATE + 1;
        constexpr double l2norm = sqrt(AR_TRGT_SMPL_RATE); // norm correction: ||x||_L2 = l2norm * ||x||_2

        BilinearInterpol interp(m_Xsi, m_Xsi, X);
        SrcFuncAccOp sfao(&interp);

        for (int n = 0; n < Error.size(); ++n) {
            RadonOperator<SrcFuncAccOp, void (double, double *, double *)>
                    Radon(sfao, circleBound, m_Sigma.col(n));
                    //Radon(sfao, squareBound, m_Sigma.col(n));

            Matrix<double, 1, numSamples> RadonData; // temporary vector for radon data
            for (int j = 0; j < m_S.cols(); ++j)
                RadonData[j] = Radon(m_S.coeffRef(j));

            Matrix<double, 1, numSamples> SchlierenData; // temporary vector for schlieren data
            SchlierenData = RadonData.cwiseProduct(RadonData);

            Error.derived()[n] = l2norm * (m_DataSet[n] - SchlierenData).norm(); // ||Y_delta - F(Xn)||_L2
        }
    }

    template <typename Derived, typename OtherDerived>
    void operator()(const int n, const EigenBase<Derived> &Xin, EigenBase<OtherDerived> &Xout)
    {
        constexpr int numSamples = 2/AR_TRGT_SMPL_RATE + 1;

        BilinearInterpol interp(m_Xsi, m_Xsi, Xin);
        SrcFuncAccOp sfao(&interp);

        RadonOperator<SrcFuncAccOp, void (double, double *, double *)>
                Radon(sfao, circleBound, m_Sigma.col(n));
                //Radon(sfao, squareBound, m_Sigma.col(n));

        Matrix<double, 1, numSamples> RadonData; // temporary vector for radon data
        for (int j = 0; j < m_S.cols(); ++j)
            RadonData[j] = Radon(m_S.coeffRef(j));

        Matrix<double, 1, numSamples> SchlierenData; // temporary vector for schlieren data
        SchlierenData = RadonData.cwiseProduct(RadonData);

        Matrix<double, 1, numSamples> Diff = m_DataSet[n] - SchlierenData; // Diff = Y_delta - F(Xn)
        Matrix<double, 1, numSamples> DiffTimesRadon = RadonData.cwiseProduct(Diff); // DiffTimesRadon = R(Xn) * Diff
        //STDOUT_MATRIX(DiffTimesRadon);

        TrgtFuncAccOp<Projection> tfao(DiffTimesRadon);
        Backprojection<TrgtFuncAccOp<Projection> > R_adjoint(tfao, m_Sigma.col(n));

        //Matrix<double, Dynamic, Dynamic> Xout(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE); // temporary matrix for backprojected data
        Xout.derived().setZero(ASYMREG_GRID_SIZE, ASYMREG_GRID_SIZE);
        for (int k = 0; k < ASYMREG_GRID_SIZE; ++k) {
            for (int l = 0; l < ASYMREG_GRID_SIZE; ++l) {
                Matrix<double, 2, 1> vec;
                vec << m_Xsi(k), m_Xsi(l);

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
                static Transform<Scalar, 2, Affine> trInv = (Translation2d(5, 5) * Scaling(5.0)).inverse();
                //STDOUT_MATRIX(trInv);

                /* translate s to target coord. system: */
                vec = trInv * vec;

                /* backproject vector: */
                Xout.derived()(k,l) = 2. * R_adjoint(vec);
            }
        }

        //STDOUT_MATRIX(Xout);
    }

private:
    const DerivedMatrix &m_Sigma;
    const DerivedVector &m_S;
    const DerivedVector &m_Xsi;
    const DerivedVector *m_DataSet;
};

// init static members:
BilinearInterpol *AsymReg::m_sourceFunc(nullptr);
RowVectorXd AsymReg::m_DataSet[AR_NUM_REC_ANGL];
Matrix<double, Dynamic, Dynamic> AsymReg::m_Result;

// function implementations:
void circleBound(double in, double *lower, double *upper)
{
    double val = sqrt(1 - in*in);
    *lower = -val;
    *upper =  val;
}

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

void AsymReg::generateDataSet(int recordingAngles, double delta, Duration *time)
{
    typedef typename MatrixXd::Index Index;

    const int angles = (recordingAngles > 0) ? recordingAngles : N;

    /* Phi:
     * ====
     * Recording angles in [0,pi] in N+1 equidistant steps,
     * but we skip the last (N+1)th step because it is the same as 0°.
     * We could use intervall [0,pi) but we will then never get the exact 90°
     * angle which means data sampling in direction of light-rays/xray.
     */
    assert(angles % 2 == 0); // only even numbers of angles are allowed, we want that 90° angle
    ArrayXd Phi = ArrayXd::LinSpaced(Sequential, angles+1, 0., M_PI).head(angles); // head(N) => take only first N entries

    /* printing list of rec. angles to stdout: */
    std::cout << "Using the following " << angles << " recording angels:" << std::endl
              << (Phi*PHI/M_PI).format(IOFormat(2, 0, "", "°, ", "", "", "", "°")) // comma-separated list
                                                                                   // and '°' after each number
              << std::endl << std::endl;

    auto t1 = hrc::now(); // Start timing (TODO: start before Phi, but exlude cout)
    /* Sigma:
     * ======
     * Recording angles Phi in euclidian coord. system.
     * Each column in Sigma represents one recording angle as target coords.
     */
    MatrixXd Sigma(2, angles);
    Sigma.row(0) = Phi.cos(); // [cos(phi_0), cos(phi_1), ... , cos(phi_n)]
    Sigma.row(1) = Phi.sin(); // [sin(phi_0), sin(phi_1), ... , sin(phi_n)]

    const Index numSamples = 2/AR_TRGT_SMPL_RATE + 1;
    static_assert(numSamples % 2, "only values for AR_TRGT_SMPL_RATE with uneven"
                                  "result of \"(2/AR_TRGT_SMPL_RATE) + 1\" are allowed");
    constexpr double l2norm = sqrt(AR_TRGT_SMPL_RATE);

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
    for (Index n = 0; n < angles; ++n) {
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
                Radon(sfao, circleBound, Sigma.col(n));
                //Radon(sfao, squareBound, Sigma.col(n));

        /* iterate over all entries in vector S: */
        for (Index j = 0; j < numSamples; ++j)
            Integral(j) = Radon(S(j)); // perform Radon-Transform for s_j

        /* finally square each entry and save as dataSet for angle phi_n: */
        m_DataSet[n] = Integral.square();
        RowVectorXd yd = m_DataSet[n];

        /* create random unit vector for data pertubation: */
        if (delta > 0.0) {
            RowVectorXd Rand = RowVectorXd::Random(2/AR_TRGT_SMPL_RATE + 1);
            //RowVectorXd RandN = Rand.normalized() * l2norm;
            m_DataSet[n] += (delta / l2norm) * Rand.normalized();
            //double diff = (m_DataSet[n] - yd).norm() * l2norm;
            //std::cout << "diff =" << diff << std::endl;
        }
    }
    auto t2 = hrc::now(); // Stop timing

    /* printing generated data y_n to stdout: */
    std::cout << "Resulting in the following disturbed (delta =" << DELTA << ") data:" << std::endl;
    for (Index n = 0; n < angles; ++n) {
        std::cout << "y_" << n << "(s) =" << std::endl
                  << m_DataSet[n] << std::endl
                  << std::endl;
    }

    std::cout << std::endl;

    if (time != nullptr)
        *time = t2 - t1;
}

double AsymReg::regularize(int recordingAngles, double delta,
                           ODE_Solver solver, int iterations, double step,
                           const PlotterSettings *pl, Duration *time)
{
    typedef typename MatrixXd::Index Index;
    typedef typename MatrixXd::Scalar Scalar;

    m_Result.setZero();

    const double h      = (step > 0.)           ? step            : H;
    const int    angles = (recordingAngles > 0) ? recordingAngles : N;

    /* prepare plotter settings: */
    ContourPlotterSettings *sett = nullptr;
    if (pl != nullptr) {
        sett = new ContourPlotterSettings;
        copySettings(pl, sett);
        sett->setTitle("regularized data", 2);
    }

    std::cout << "Solving ODE with ";

    switch (solver) {
    case Euler:
        std::cout << "Euler (direct)";
        break;
    case Midpoint:
        std::cout << "Midpoint (2nd order)";
        break;
    case RungeKutta:
        std::cout << "Runge Kutta (4th order)";
        break;
    }

    std::cout << " method:" << std::endl
              << "  -> step size h = " << h << std::endl
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

    ArrayXd Phi = ArrayXd::LinSpaced(Sequential, angles+1, 0., M_PI).head(angles); // head(N) => take only first N entries
    MatrixXd Sigma(2, angles);
    Sigma.row(0) = Phi.cos(); // [cos(phi_0), cos(phi_1), ... , cos(phi_n)]
    Sigma.row(1) = Phi.sin(); // [sin(phi_0), sin(phi_1), ... , sin(phi_n)]

    const Index numSamples = 2/AR_TRGT_SMPL_RATE + 1;
    RowVectorXd S = VectorXd::LinSpaced(Sequential, numSamples, -1., 1.);

    RowVectorXd Error(angles);
    Error.setConstant(-1.0);

    int run = 0;
    int max = (iterations > 0) ? iterations : T;
    do {
        DerivateOperator<MatrixXd, RowVectorXd> derivs(Sigma, S, Xsi, &m_DataSet[0]);
        Matrix<double , Dynamic, Dynamic> dXdt[angles];

        for (int n = 0; n < angles; ++n) {
            dXdt[n].resizeLike(Xn);

            derivs(n, Xn, dXdt[n]);
            //STDOUT_MATRIX(dXdt);
        }

        switch (solver) {
        case Euler:
            ODE::euler(angles, Xn, &dXdt[0], h, Xdot, derivs);
            break;
        case Midpoint:
            ODE::rk2(angles, Xn, &dXdt[0], h, Xdot, derivs);
            break;
        case RungeKutta:
            ODE::rk4(angles, Xn, &dXdt[0], h, Xdot, derivs);
            break;
        }

        derivs.error(Xdot, Error);
        double err = Error.mean();

        std::string itrStr;
        if (iterations == 0) { // discrepancy principle is used
            itrStr = "Iteration no. " + std::to_string(run + 1)
                     + " (of max " + std::to_string(max) + ")";
        } else {
            itrStr = "Iteration no. " + std::to_string(run + 1)
                     + " / " + std::to_string(max);
        }

        std::cout << itrStr << " with error = " << err << std::endl;

        if (isnan(err)) {
            std::cout << "Something bad happend! Stopping now!!!" << std::endl;
            break; // we take the last result, because it is probably
                   // the better one (with an error != NAN)
        }

        Xn = Xdot; // use regularized data for next iteration step (also as result)

        /* discrepancy principle: */
        if ((iterations == 0) && (err <= delta * TAU)) {
            std::cout << "Stopping due to discrepancy level reached!" << std::endl;
            break; // quit while(), err is small enough
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
    return Error.mean();
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
