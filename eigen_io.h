#ifndef EIGEN_IO_H_
#define EIGEN_IO_H_

#include <stdexcept>
#include <istream>
#include <sstream>

#include <Eigen/Core>

namespace Eigen {

class IOFormatException : public std::runtime_error
{
public:
    enum ExceptionType {
        CoeffSeparatorException,
        RowSeparatorException,
        MatPrefixException,
        MatSuffixException
    };

    IOFormatException(ExceptionType exception, const std::string &expectedDelim, const std::string &foundDelim,
                      int rowIndex = -1, int colIndex = -1)
        : runtime_error("")
    {
        std::ostringstream what;
        what << "Eigen::IOFormat import error" << " while parsing ";

        switch (exception) {
        case CoeffSeparatorException:
            what << "coeffSeparator after element ("
                 << rowIndex << ',' << colIndex << ')';
            break;
        case RowSeparatorException:
            what << "rowSeparator at the end of row " << rowIndex;
            break;
        case MatPrefixException:
            what << "matPrefix";
            break;
        case MatSuffixException:
            what << "matSuffix";
            break;
        }

        what << ":\n\t expected delimeter '";
        // escape '\n' and '\t' chars in delimeter:
        for (int i = 0; i < expectedDelim.size(); ++i) {
            if (expectedDelim[i] == '\n')
                what << "\\n";
            else if (expectedDelim[i] == '\t')
                what << "\\t";
            else
                what << expectedDelim[i];
        }
        what << "' but found '";
        // escape '\n' and '\t' chars in delimeter:
        for (int i = 0; i < foundDelim.size(); ++i) {
            if (foundDelim[i] == '\n')
                what << "\\n";
            else if (foundDelim[i] == '\t')
                what << "\\t";
            else
                what << foundDelim[i];
        }
        what << "' instead";

        // Insert dynamically created what-string into runtime_error class.
        // This is easier than overloading what(). First of all it is less code.
        // But more important, it preserves the string after throwing the
        // exception. I don't know exactly why, but it can get lost and you end
        // with what() returing "".
        // http://stackoverflow.com/questions/2614113/return-a-dynamic-string-from-stdexceptions-what
        static_cast<std::runtime_error &>(*this) = std::runtime_error(what.str());
    }
};

// istream-operator to import data from stream/file:
template<typename Derived>
inline std::istream &operator>>(std::istream &s, Derived &m)
{ return import_matrix(s, m, EIGEN_DEFAULT_IO_FORMAT); }

// const-variant of istream-operator
template<typename Derived>
inline std::istream &operator>>(std::istream &s, const Derived &m)
{ return import_matrix(s, const_cast<Derived &>(m), EIGEN_DEFAULT_IO_FORMAT); }

// import matrix/vector from stream/file:
template<typename Derived>
std::istream &import_matrix(std::istream &s, Derived &m, const IOFormat &fmt)
{
    typedef typename Derived::Scalar Scalar;
    typedef typename Derived::Index Index;

    // m needs to have a fixed or known size, otherwise import will fail:
    eigen_assert((m.ColsAtCompileTime != Dynamic) || m.cols());
    eigen_assert((m.RowsAtCompileTime != Dynamic) || m.rows());

    // string to compare stream's content with fmt:
    std::string rowSepDelim(fmt.rowSeparator.size(), '\0');
    std::string coeffSepDelim(fmt.coeffSeparator.size(), '\0');
    std::string otherDelim; // matPrefix, matSuffix

    // Matrix Prefix:
    otherDelim.resize(fmt.matPrefix.size()); // automatically filled with '\0'
    s.read(&otherDelim[0], fmt.matPrefix.size());
    if (fmt.matPrefix.compare(otherDelim)) {
#ifndef EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS
        throw IOFormatException(IOFormatException::MatPrefixException,
                                fmt.matPrefix, otherDelim);
#endif // EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS
        goto fail_out;
    }

    for (Index r = 0; r < m.rows(); ++r) {
        if (r) {
            // Row Separator:
            s.read(&rowSepDelim[0], fmt.rowSeparator.size());
            if (fmt.rowSeparator.compare(rowSepDelim)) {
#ifndef EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS
                throw IOFormatException(IOFormatException::RowSeparatorException,
                                        fmt.rowSeparator, rowSepDelim, r);
#endif // EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS
                goto fail_out;
            }

            // Row Spacer:
            s.ignore(fmt.rowSpacer.size()); // just skip the size of a rowSpacer
        }

        // Row Prefix:
        s.ignore(fmt.rowPrefix.size()); // just skip appropriate length of chars

        // iterate over col's to read actual data:
        for (Index c = 0; c < m.cols(); ++c) {
            if (c) {
                // Coeff Separator:
                s.read(&coeffSepDelim[0], fmt.coeffSeparator.size());
                if (fmt.coeffSeparator.compare(coeffSepDelim)) {
#ifndef EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS
                    throw IOFormatException(IOFormatException::CoeffSeparatorException,
                                            fmt.coeffSeparator, coeffSepDelim, r, c);
#endif // EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS
                    goto fail_out;
                }
            }

            // Parse value:
            Scalar val;
            s >> val;

            // If anything went wrong above at parsing 'val', then it is not a IOFormat
            // problem, but rather a bad stream. So we don't throw an exection here.
            // The user can still do this by calling
            //   istream.exceptions(std::io_base::failbit | std::io_base::badbit);
            // beforehand and catch for 'const std::ios_base::failure &e'.
            if (s.fail())
                return s; // failbit or badbit set already, so just return from function

            m(r, c) = val; // insert value
        }

        // Row Suffix:
        s.ignore(fmt.rowSuffix.size()); // just skip appropriate length of chars
    }

    // Matrix Suffix:
    otherDelim.resize(fmt.matSuffix.size());
    s.read(&otherDelim[0], fmt.matSuffix.size());
    if (fmt.matSuffix.compare(otherDelim)) {
#ifndef EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS
        throw IOFormatException(IOFormatException::MatSuffixException,
                                fmt.matSuffix, otherDelim);
#endif // EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS
        goto fail_out;
    }

    return s;

fail_out:
    s.setstate(std::ios::failbit);
    return s;
}

template<typename Derived>
class WithFormat2
{
public:
    WithFormat2(Derived &matrix, const IOFormat &format)
        : m_matrix(matrix),
          m_format(format)
    {}

    inline friend std::istream &operator>>(std::istream &s, const WithFormat2 &wf)
    { return import_matrix(s, wf.m_matrix, wf.m_format); }

protected:
    Derived &m_matrix;
    IOFormat m_format;
};

template<typename Derived>
inline WithFormat2<Derived> DenseBase<Derived>::format2(const IOFormat &fmt)
{ return WithFormat2<Derived>(derived(), fmt); }

} // namespace Eigen

#endif // EIGEN_IO_H_
