#ifndef CONSTANTS_H_
#define CONSTANTS_H_

constexpr double H   = 5;             /**< ODE solver: maximum step size */
constexpr int    T   = 150;              /**< ODE solver: maximum iterations */
constexpr int    N   = AR_NUM_REC_ANGL; /**< Radontransform: maximum number of recording angles used */
constexpr double PHI = 180.0;           /**< Radontransform: highest possible recording angle [deg] */
constexpr double X0_C = 0.01;           /**< ODE Solver: constant used as initial value for X0 matrix */
constexpr double DELTA = AR_DELTA;      /**< Inverse Problem: data pertubation */
constexpr double TAU = 2.0;             /**< Inverse Problem: morozov's diskrepancy */

#endif // CONSTANTS_H_
