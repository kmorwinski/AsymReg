#ifndef CONSTANTS_H_
#define CONSTANTS_H_

constexpr double H   = .01;             // Euler step
constexpr int    T   = 6;               // maximum iterations for Euler method
constexpr int    N   = AR_NUM_REC_ANGL; // number of recording angles
constexpr double PHI = 180.;            // maximum rec. angle
constexpr double X0_C = 0.1;            // constant used as initial value for X0 Matrix

#endif // CONSTANTS_H_
