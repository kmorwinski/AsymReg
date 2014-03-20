#include "interpol.h"

#include <algorithm>
#include <cmath>

BaseInterpol::BaseInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y, int m)
    : m_n(x.size()),
      m_m(m),
      m_kLast(0),
      m_cor(0),
      m_x(x.data()),
      m_y(y.data())
{
    m_dj = std::min(1, (int)pow((double)m_n, .25));
}

int BaseInterpol::hunt(const double x)
{
    if ((m_n < 2) || (m_m < 2) || (m_m > m_n))
        throw("hunt size error");

    int kLower = m_kLast;
    int kMiddle;
    int kUpper;
    bool ascending = (m_x[m_n - 1] >= m_x[0]);
    if ((kLower < 0) || (kLower > m_n - 1)) {
        kLower = 0;
        kUpper = m_n - 1;
    } else {
        int inc = 1;
        if (x >= m_x[kLower] == ascending) { // hunt up
            while (1) {
                kUpper = kLower + inc;
                if (kUpper >= m_n - 1) {
                    kUpper = m_n -1;
                    break;
                } else if (x < m_x[kUpper] == ascending)
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
                } else if (x >= m_x[kLower] == ascending)
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
        if (x >= m_x[kMiddle] == ascending)
            kLower = kMiddle;
        else
            kUpper = kMiddle;
    }

    m_cor = std::abs(kLower - m_kLast) > m_dj ? 0 : 1;
    m_kLast = kLower;

    return std::max(0, std::min(m_n - m_m, kLower - ((m_m - 2) >> 1)));
}

double BaseInterpol::interpol(double x)
{
    int kLower = /*m_cor ? hunt(x) :*/ locate(x);
    return rawinterpol(kLower, x);
}

int BaseInterpol::locate(const double x)
{
    if ((m_n < 2) || (m_m < 2) || (m_m > m_n))
        throw("locate size error");

    int kLower = 0;
    int kUpper = m_n - 1;
    bool ascending = (m_x[m_n - 1] >= m_x[0]);

    int kMid;
    while(kUpper - kLower > 1) {
        kMid = (kUpper + kLower) >> 1;
        if (x >= m_x[kMid] == ascending)
            kLower = kMid;
        else
            kUpper = kMid;
    }

    m_cor = std::abs(kLower - m_kLast) > m_dj ? 0 : 1;
    m_kLast = kLower;

    return std::max(0, std::min(m_n - m_m, kLower - ((m_m - 2) >> 1)));
}

LinearInterpol::LinearInterpol(const Eigen::VectorXd &x, const Eigen::VectorXd &y)
    : BaseInterpol(x, y, 2)
{
}

double LinearInterpol::rawinterpol(int k, double x) const
{
    if (m_x[k] == m_x[k + 1])
        return m_y[k];

    return m_y[k] + (x - m_x[k]) / (m_x[k + 1] - m_x[k]) * (m_y[k+1] - m_y[k]);
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
        u(0) = (3.0/(m_x[1] - m_x[0])) * ((m_y[1] - m_y[0]) / (m_x[1] - m_x[0]) - yp1);
    }

    for (int i = 0; i < n; ++i) {
        double sig = (m_x[i] - m_x[i-1]) / (m_x[i+1] - m_x[i-1]);
        double p = sig * m_y2.coeff(i-1) + 2.0;
        m_y2(i) = (sig - 1.0) / p;
        u(i) = (m_y[i+1] - m_y[i]) / (m_x[i+1] - m_x[i]) - (m_y[i] - m_y[i-1]) / (m_x[i] - m_x[i-1]);
        u(i) = (6.0 * u.coeff(i) / (m_x[i+1] - m_x[i-1]) - sig * u.coeff(i-1)) / p;
    }

    double qn;
    double un;
    if (ypn > 0.99e99) {
        qn = .0;
        un = .0;
    } else {
        qn = .5;
        un = (3.0/(m_x[n-1] - m_x[n-2])) * (ypn - (m_y[n-1] - m_y[n-2]) / (m_x[n-1] - m_x[n-2]));
    }

    m_y2(n-1) = (un - qn * u.coeff(n-2)) / (qn * m_y2.coeff(n-2) + 1.0);
    for (int k = n-2; k >= 0; --k)
        m_y2(k) = m_y2.coeff(k) * m_y2.coeff(k+1) + u.coeff(k);
}

double SplineInterpol::rawinterpol(int k, double x) const
{
    int kLow = k;
    int kHigh = k + 1;
    double h = m_x[kHigh] - m_x[kLow];
    if (h == .0)
        throw("Bad input to routine spline-interpol");

    double a = (m_x[kHigh] - x) / h;
    double b = (x - m_x[kLow]) / h;

    double y = a * m_y[kLow] + b * m_y[kHigh] +
            ((a * a * a - a) * m_y2.coeff(kLow) +
             (b * b * b -b) * m_y2.coeff(kHigh)) * (h * h) / 6.0;

    return y;
}
