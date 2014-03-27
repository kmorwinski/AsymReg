#ifndef EIGEN_H_
#define EIGEN_H_

// set CSV Format as default:
#define EIGEN_DEFAULT_IO_FORMAT IOFormat(FullPrecision, 0, ",") // <-- ',' without space is important

#include <Eigen/Dense>

#include <istream>

namespace Eigen {

    template<typename Derived>
    std::istream &operator>>(std::istream &s, DenseBase<Derived> &m)
    {
        return read_matrix(s, m, EIGEN_DEFAULT_IO_FORMAT);
    }

    template<typename Derived>
    std::istream &read_matrix(std::istream &s, DenseBase<Derived> &m, const IOFormat &fmt)
    {
        typedef typename Derived::Scalar Scalar;

        typedef typename Derived::Index Index;
        const Index cols = m.cols();
        const Index rows = m.rows();
        eigen_assert((m.ColsAtCompileTime != Dynamic) || cols);
        eigen_assert((m.RowsAtCompileTime != Dynamic) || rows);

        const int coeffSeparatorSize = fmt.coeffSeparator.size();
        const int rowSeparatorSize = fmt.rowSeparator.size();
        const int matPrefixSize = fmt.matPrefix.size();
        const int matSuffixSize = fmt.matSuffix.size();
        const int rowSpacerSize = fmt.rowSpacer.size();
        const int rowPrefixSize = fmt.rowPrefix.size();
        const int rowSuffixSize = fmt.rowSuffix.size();

        // strings to compare stream's content with fmt:
        std::string coeffSeparator(coeffSeparatorSize, '\0');
        std::string rowSeparator(rowSeparatorSize, '\0');
        std::string matPrefix(matPrefixSize, '\0');
        std::string matSuffix(matSuffixSize, '\0');
        std::string rowPrefix(rowPrefixSize, '\0');
        std::string rowSuffix(rowSuffixSize, '\0');

        // Matrix Prefix:
        s.read(&matPrefix[0], matPrefixSize);
        eigen_assert(fmt.matPrefix.compare(matPrefix) == 0);

        for (Index r = 0; r < rows; ++r) {
            if (r) {
                // Row Separator:
                s.read(&rowSeparator[0], rowSeparatorSize);
                eigen_assert(fmt.rowSeparator.compare(rowSeparator) == 0);

                // Row Spacer:
                s.ignore(rowSpacerSize);
            }

            // Row Prefix:
            s.read(&rowPrefix[0], rowPrefixSize);
            eigen_assert(fmt.rowPrefix.compare(rowPrefix) == 0);

            // iterate over col's to read actual data:
            for (Index c = 0; c < cols; ++c) {
                if (c) {
                    // Coeff Separator:
                    s.read(&coeffSeparator[0], coeffSeparatorSize);
                    eigen_assert(fmt.coeffSeparator.compare(coeffSeparator) == 0);
                }

                eigen_assert(s.good());
                Scalar val;
                s >> val;
                m(r, c) = val;
            }

            // Row Suffix:
            s.read(&rowSuffix[0], rowSuffixSize);
            eigen_assert(fmt.rowSuffix.compare(rowSuffix) == 0);
        }

        // Matrix Suffix:
        s.read(&matSuffix[0], matSuffixSize);
        eigen_assert(fmt.matSuffix.compare(matSuffix) == 0);

        return s;
    }

} // namespace Eigen

#endif // EIGEN_H_
