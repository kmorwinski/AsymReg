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

} // namespace ODE

#endif // ODE_H_
