#ifndef INTERPOL_H_
#define INTERPOL_H_

#include "eigen.h"

class BaseInterpol
{
public:
    BaseInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y, int m);

    double interpol(double x) const;

protected:
    virtual double rawinterpol(int k, double x) const = 0;

    const double *xData() const;
    const double *yData() const;

private:
    int hunt(const double x) const;
    int bisect(const double x) const;

    Eigen::VectorXd m_x;
    Eigen::VectorXd m_y;
    int m_indexThreshold;
    mutable bool m_localIndex;
    mutable int m_lastIndex;
    int m_M;
    int m_N;

    friend class BilinearInterpol;
};

class LinearInterpol : public BaseInterpol
{
public:
    LinearInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y);

protected:
    virtual double rawinterpol(int k, double x) const;
};

class SplineInterpol : public BaseInterpol
{
public:
    SplineInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y, double yp1 = 1.e99, double ypn = 1.e99);

protected:
    virtual double rawinterpol(int k, double x) const;

private:
    void sety2(const double *x, const double *y, double yp1, double ypn);

    Eigen::VectorXd m_y2;
};

class BilinearInterpol
{
public:
    BilinearInterpol(const Eigen::VectorXd &x1, const Eigen::VectorXd &x2, const Eigen::MatrixXd &y);

    double interpol(double x1, double x2) const;

private:
    LinearInterpol m_x1interpol;
    LinearInterpol m_x2interpol;
    Eigen::MatrixXd m_y;
};

#endif // INTERPOL_H_
