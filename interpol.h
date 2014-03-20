#ifndef INTERPOL_H_
#define INTERPOL_H_

#include "eigen.h"

class BaseInterpol
{
public:
    BaseInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y, int m);

    double interpol(double x);

protected:
    virtual double rawinterpol(int k, double x) const = 0;

    const double *m_x;
    const double *m_y;

private:
    int hunt(const double x);
    int locate(const double x);

    int m_n;
    int m_m;
    int m_kLast;
    int m_cor;
    int m_dj;

};

class LinearInterpol : public BaseInterpol
{
public:
    LinearInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y);

protected:
    virtual double rawinterpol(int k, double x) const;
};

#endif // INTERPOL_H_
