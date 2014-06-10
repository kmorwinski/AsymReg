#ifndef BACKPROJECTION_H_
#define BACKPROJECTION_H_

/**
 * Adjoint Radon Operator
 */
template<typename Func>
class Backprojection {
public:
    Backprojection(Func &function, /*int gridSize,*/ const Eigen::Vector2d &sigma)
        : m_Func(function),
          m_Sigma(sigma)//,
          //m_gridSize(size)
    {}

    template<typename Derived>
    typename Derived::Scalar operator()(const EigenBase<Derived> &xy)
    {
        typedef typename Derived::Index Index;
        typedef typename Derived::Scalar Scalar;

        static Transform<Scalar, 2, Affine> trInv = (Translation2d(5, 5) * Scaling(3.0)).inverse();
        Derived rs = trInv * xy.derived();
        Scalar s = m_Sigma.transpose() * rs; //xy.derived();
        Scalar ret = m_Func(s);

        return ret;
    }

private:
    Func &m_Func;
    Eigen::Vector2d m_Sigma;
    //int m_gridSize;
};

#endif // BACKPROJECTION_H_
