#include "interpol.h"

#include <algorithm>
#include <cmath>

BaseInterpol::BaseInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y, int m)
    : m_N(x.size()),
      m_M(m),
      m_lastIndex(0),
      m_localIndex(0),
      m_x(x),
      m_y(y)
{
    m_indexThreshold = std::min(1, (int)pow((double)m_N, .25));
}

int BaseInterpol::hunt(double x) const
{
    if ((m_N < 2) || (m_M < 2) || (m_M > m_N))
        throw("hunt size error");

    const double *xd = m_x.data();
    int kLower = m_lastIndex;
    int kMiddle;
    int kUpper;
    bool ascending = (xd[m_N - 1] >= xd[0]);
    if ((kLower < 0) || (kLower > m_N - 1)) {
        kLower = 0;
        kUpper = m_N - 1;
    } else {
        int inc = 1;
        if (x >= xd[kLower] == ascending) { // hunt up
            while (1) {
                kUpper = kLower + inc;
                if (kUpper >= m_N - 1) {
                    kUpper = m_N -1;
                    break;
                } else if (x < xd[kUpper] == ascending)
                    break;
                else {
                    kLower = kUpper;
                    inc += inc;
                }
            }
        } else { // hunt down
            kUpper = kLower;
            while (1) {
                kLower = kLower - inc;
                if (kLower <= 0) {
                    kLower = 0;
                    break;
                } else if (x >= xd[kLower] == ascending)
                    break;
                else {
                    kUpper = kLower;
                    inc += inc;
                }
            }
        }
    }

    while (kUpper - kLower > 1) {
        kMiddle = (kUpper + kLower) >> 1;
        if (x >= xd[kMiddle] == ascending)
            kLower = kMiddle;
        else
            kUpper = kMiddle;
    }

    m_localIndex = std::abs(kLower - m_lastIndex) <= m_indexThreshold;
    m_lastIndex = kLower;

    return std::max(0, std::min(m_N - m_M, kLower - ((m_M - 2) >> 1)));
}

double BaseInterpol::interpol(double x) const
{
    int kLower = m_localIndex ? hunt(x) : bisect(x);
    return rawinterpol(kLower, x);
}

int BaseInterpol::bisect(double x) const
{
    const double *xd = m_x.data();

    if ((m_N < 2) || (m_M < 2) || (m_M > m_N))
        throw("locate size error");

    int kLower = 0;
    int kUpper = m_N - 1;
    bool ascending = (xd[m_N - 1] >= xd[0]);

    int kMid;
    while(kUpper - kLower > 1) {
        kMid = (kUpper + kLower) >> 1;
        if (x >= xd[kMid] == ascending)
            kLower = kMid;
        else
            kUpper = kMid;
    }

    m_localIndex = std::abs(kLower - m_lastIndex) <= m_indexThreshold;
    m_lastIndex = kLower;

    return std::max(0, std::min(m_N - m_M, kLower - ((m_M - 2) >> 1)));
}

const double *BaseInterpol::xData() const
{
    return m_x.data();
}

const double *BaseInterpol::yData() const
{
    return m_y.data();
}

BilinearInterpol::BilinearInterpol(const Eigen::VectorXd &x1, const Eigen::VectorXd &x2, const Eigen::MatrixXd &y)
    : m_x1interpol(x1, x1),
      m_x2interpol(x2, x2),
      m_y(y)
{
}

double BilinearInterpol::interpol(double x1, double x2) const
{
    int i = /*m_x1interpol.m_localIndex ? m_x1interpol.hunt(x1) :*/ m_x1interpol.bisect(x1);
    int j = /*m_x2interpol.m_localIndex ? m_x2interpol.hunt(x2) :*/ m_x2interpol.bisect(x2);

    double t = (x1 - m_x1interpol.m_x[i])
             / (m_x1interpol.m_x[i+1] - m_x1interpol.m_x[i]);
    double u = (x2 - m_x2interpol.m_x[j])
             / (m_x2interpol.m_x[j+1] - m_x2interpol.m_x[j]);
    double y = (1.-t) * (1.-u) * m_y.coeff(i  ,j  )
             + t      * (1.-u) * m_y.coeff(i+1,j  )
             + (1.-t) *     u  * m_y.coeff(i  ,j+1)
             + t      *     u  * m_y.coeff(i+1,j+1);

    return y;
}

LinearInterpol::LinearInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y)
    : BaseInterpol(x, y, 2)
{
}

double LinearInterpol::rawinterpol(int k, double x) const
{
    const double *xd = xData();
    const double *yd = yData();

    if (xd[k] == xd[k + 1])
        return yd[k];

    return yd[k] + (x - xd[k]) / (xd[k + 1] - xd[k]) * (yd[k+1] - yd[k]);
}

SplineInterpol::SplineInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y, double yp1, double ypn)
    : BaseInterpol(x, y, 2),
      m_y2(x.size())
{
    sety2(x.data(), y.data(), yp1, ypn);
}

void SplineInterpol::sety2(const double *x, const double *y, double yp1, double ypn)
{
    const int n = m_y2.size();
    Eigen::VectorXd u(n);
    if (yp1 > 0.99e99) {
        m_y2(0) = .0;
        u(0) = .0;
    } else {
        m_y2(0) = -.5;
        u(0) = (3.0/(x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
    }

    for (int i = 0; i < n; ++i) {
        double sig = (x[i] - x[i-1]) / (x[i+1] - x[i-1]);
        double p = sig * m_y2.coeff(i-1) + 2.0;
        m_y2(i) = (sig - 1.0) / p;
        u(i) = (y[i+1] - y[i]) / (x[i+1] - x[i]) - (y[i] - y[i-1]) / (x[i] - x[i-1]);
        u(i) = (6.0 * u.coeff(i) / (x[i+1] - x[i-1]) - sig * u.coeff(i-1)) / p;
    }

    double qn;
    double un;
    if (ypn > 0.99e99) {
        qn = .0;
        un = .0;
    } else {
        qn = .5;
        un = (3.0/(x[n-1] - x[n-2])) * (ypn - (y[n-1] - y[n-2]) / (x[n-1] - x[n-2]));
    }

    m_y2(n-1) = (un - qn * u.coeff(n-2)) / (qn * m_y2.coeff(n-2) + 1.0);
    for (int k = n-2; k >= 0; --k)
        m_y2(k) = m_y2.coeff(k) * m_y2.coeff(k+1) + u.coeff(k);
}

double SplineInterpol::rawinterpol(int k, double x) const
{
    int kLow = k;
    int kHigh = k + 1;
    const double *xd = xData();
    double h = xd[kHigh] - xd[kLow];
    if (h == .0)
        throw("Bad input to routine spline-interpol");

    double a = (xd[kHigh] - x) / h;
    double b = (x - xd[kLow]) / h;

    const double *yd = yData();
    double y = a * yd[kLow] + b * yd[kHigh] +
            ((a * a * a - a) * m_y2.coeff(kLow) +
             (b * b * b -b) * m_y2.coeff(kHigh)) * (h * h) / 6.0;

    return y;
}
