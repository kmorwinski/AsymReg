#ifndef ODE_H_
#define ODE_H_

namespace ODE {

// Euler (direct)
template <typename Derived, typename DerivsFunc>
void euler(const int angles, const EigenBase<Derived> &X, const Derived *dXdt,
           const typename Derived::Scalar h,
           EigenBase<Derived> &Xout, DerivsFunc &derivs)
{
    typedef typename Derived::Scalar Scalar;
    typedef typename Derived::Index Index;

    Xout.derived() = X;

    for (Index i = 0; i < angles; ++i)
        Xout.derived() += (h / Scalar(angles)) * dXdt[i];
}

// Runge-Kutta 2th order
template <typename Derived, typename DerivsFunc>
void rk2(const int angles, const EigenBase<Derived> &X, const Derived *dXdt,
           const typename Derived::Scalar h,
           EigenBase<Derived> &Xout, DerivsFunc &derivs)
{
    typedef typename Derived::Scalar Scalar;
    typedef typename Derived::Index Index;

    Xout.derived() = X;

    Derived K1, K2;

    for (int i = 0; i < angles; ++i) {
        K1 = .5 * h * dXdt[i];
        derivs(i, K1 + X.derived(), K2);
        Xout.derived() += (h / Scalar(angles)) * K2;
    }
}

// Runge-Kutta 4th order
template <typename Derived, typename DerivsFunc>
void rk4(const int angles, const EigenBase<Derived> &X, const Derived *dXdt,
           const typename Derived::Scalar h,
           EigenBase<Derived> &Xout, DerivsFunc &derivs)
{
    typedef typename Derived::Scalar Scalar;

    Xout.derived() = X;

    Derived K1, K2, K3, K4;

    for (int i = 0; i < angles; ++i) {
        K1 = .5 * h * dXdt[i];
        derivs(i,          K1 + X.derived(), K2);
        derivs(i, .5 * h * K2 + X.derived(), K3);
        derivs(i,      h * K3 + X.derived(), K4);

        Xout.derived() += (1./6.*(K1 + h*K4) + 1./3.*(K2 + K3)) / Scalar(angles);
    }
}

} // namespace ODE

#endif // ODE_H_
