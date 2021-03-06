#ifndef RADONOPERATOR_H_
#define RADONOPERATOR_H_

#include "eigen.h"
#include "asymreg.h"

#include <map>

/**
 * @brief Radon Operator
 * This template class behaves like the Radon Operator.
 */
template <typename Func, typename Boundary>
class RadonOperator
{
public:
    RadonOperator(Func &function, /*int gridSize,*/
                  Boundary &rAxisBoundaryFunction, const Eigen::Vector2d &sigma)
        : m_Func(function),
          /*m_gridSize(gridSize),*/
          m_Boundary(rAxisBoundaryFunction),
          m_Sigma(sigma)
    {}

    double operator()(const double s)
    {
        /* get integration boundaries for r-axis: */
        double lower, upper;
        m_Boundary(s, &lower, &upper);

        /* R:
         * ==
         * Discrete samples of r-axis (target coord. system) in the given
         * interval [lower, upper].
         * The basis for this sampling is the value defined in AR_TRGT_SMPL_RATE
         * R_i = lower + i * numSamples for i=0, 1, 2, ...,  and where
         * numSamples = 1 + ((|lower| + |upper|)/AR_TRGT_SMPL_RATE)
         *
         * TODO: use gridSize (like Plotter) as calculation basis
         */
        int numSamples = 1 + (std::abs(lower) + std::abs(upper))/AR_TRGT_SMPL_RATE;
        RowVectorXd R = VectorXd::LinSpaced(Sequential, numSamples, lower, upper);

        //std::cout << "R =" << std::endl << R << std::endl << std::endl;

        /* Rot90:
         * ======
         * Rotation of 90° to get perpendicular angle.
         */
        static Matrix<double, 2, 2> Rot90 = Rotation2Dd(M_PI_2).toRotationMatrix(); // M_PI/2

        /* Line:
         * =====
         * First part of equation:
         *  - Standard line which crosses the origin: r*sigma_n^{\perp} for r=lower,...,upper
         *
         * Second part of equation:
         *  - translate Line along projection of s * sigma_n
         */
        Matrix<double, 2, Dynamic> Line = (Rot90 * m_Sigma * R).colwise() + s * m_Sigma;

        //std::cout << "Line(s=" << s << ") = " << std::endl
        //          << Line << std::endl;

        /* gather data along Line: */
        Matrix<double, 1, Dynamic> IntData = m_Func(Line);

        //std::cout << "data to be integrated = " << std::endl
        //          << IntData[n] << std::endl << std::endl;

        /* Trapez vectors are cached in map using size (numSamples) as key: */
        static std::map<int, Matrix<double, Dynamic, 1> > trapezMap;
        auto it = trapezMap.find(numSamples);
        if (it == trapezMap.end()) { /* no Trapez vector with appropriate size? ... */
            /* ... then we construct one & add it to map */
            Matrix<double, Dynamic, 1> _Trapez;
            _Trapez.setConstant(numSamples, 1./numSamples);
            _Trapez(0) *= .5;
            _Trapez(numSamples-1) *= .5;

            it = trapezMap.insert(it, std::make_pair(numSamples, _Trapez));
        }

        /* Trapez:
         * =======
         * Vector containing the coefficients for the extended trapezoidal rule.
         * Trapez = [1/(2*numSamples), 1/numSamples, ..., 1/numSamples, 1/(2*numSamples)]
         * (Trapez-vectors are cached in std::map to speed up)
         */
        Matrix<double, Dynamic, 1> *Trapez = &it->second;

        /* calculate trapezoidal: */
        double ret = IntData * *Trapez; // mult. vectors [1 x numSamples]*[numSamples x 1]^T

        //std::cout << "integral =" << std::endl
        //          << ret << std::endl << std::endl;

        return ret;
    }

private:
    Func &m_Func;
    Boundary &m_Boundary;
    Eigen::Vector2d m_Sigma;
    //int m_gridSize;
};

#endif // RADONOPERATOR_H_
