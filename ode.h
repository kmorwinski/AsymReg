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

} // namespace ODE

#endif // ODE_H_
