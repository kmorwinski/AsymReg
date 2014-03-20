#include "interpol.h"

#include <algorithm>
#include <cmath>

BaseInterpol::BaseInterpol(const VectorXd &x, const double *y, int m)
    : m_n(x.size()),
      m_m(m),
      m_kLast(0),
      m_cor(0),
      m_x(&x),
      m_y(y)
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
    bool ascending = (m_x->coeff(m_n - 1) >= m_x->coeff(0));
    if ((kLower < 0) || (kLower > m_n - 1)) {
        kLower = 0;
        kUpper = m_n - 1;
    } else {
        int inc = 1;
        if (x >= m_x->coeff(kLower) == ascending) { // hunt up
            while (1) {
                kUpper = kLower + inc;
                if (kUpper >= m_n - 1) {
                    kUpper = m_n -1;
                    break;
                } else if (x < m_x->coeff(kUpper) == ascending)
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
                } else if (x >= m_x->coeff(kLower) == ascending)
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
        if (x >= m_x->coeff(kMiddle) == ascending)
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
    int kLower = m_cor ? hunt(x) : locate(x);
    return rawinterpol(kLower, x);
}

int BaseInterpol::locate(const double x)
{
    if ((m_n < 2) || (m_m < 2) || (m_m > m_n))
        throw("locate size error");

    int kLower = 0;
    int kUpper = m_n - 1;
    bool ascending = (m_x->coeff(m_n - 1) >= m_x->coeff(0));

    int kMid;
    while(kUpper - kLower > 1) {
        kMid = (kUpper + kLower) >> 1;
        if (x >= m_x->coeff(kMid) == ascending)
            kLower = kMid;
        else
            kUpper = kMid;
    }

    m_cor = std::abs(kLower - m_kLast) > m_dj ? 0 : 1;
    m_kLast = kLower;

    return std::max(0, std::min(m_n - m_m, kLower - ((m_m - 2) >> 1)));
}

LinearInterpol::LinearInterpol(const VectorXd &x, const VectorXd &y)
    : BaseInterpol(x, y.data(), 2)
{
}

double LinearInterpol::rawinterpol(int k, double x)
{
    if (m_x->coeff(k) == m_x->coeff(k + 1))
        return m_y[k];

    return m_y[k] * ((x - m_x->coeff(k)) / m_x->coeff(k + 1) - m_x->coeff(k)) * (m_y[k+1] - m_y[k]);
}

