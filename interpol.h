#ifndef INTERPOL_H_
#define INTERPOL_H_

#include "eigen.h"

using namespace Eigen;

class BaseInterpol
{
public:
    BaseInterpol(const VectorXd &x, const double *y, int m);

    double interpol(double x);

protected:
    virtual double rawinterpol(int k, double x) = 0;

    const VectorXd *m_x;
    const double *m_y;


private:
    int hunt(const double x);
    int locate(const double x);

    int m_n;
    int m_m;
    int m_kLast;
    int m_cor;
    int m_dj;

    //const double *xx;
};

class LinearInterpol : public BaseInterpol
{
public:
    LinearInterpol(const VectorXd &x, const VectorXd &y);

protected:
    virtual double rawinterpol(int k, double x);
};

#endif // INTERPOL_H_
