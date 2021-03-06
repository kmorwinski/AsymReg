#ifndef UNUSED_CODE_H_
#define UNUSED_CODE_H_

#include <eigen3/Eigen/Dense>

#include <istream>

namespace Eigen {
    template<typename Derived>
    std::istream &operator>>(std::istream &s, DenseBase<Derived> &m)
    {
        return read_dynamic_matrix(s, m, EIGEN_DEFAULT_IO_FORMAT);
    }

    template<typename Derived>
    std::istream &read_dynamic_matrix(std::istream &s, DenseBase<Derived> &_m, const IOFormat &fmt)
    {
        typedef typename Derived::Scalar Scalar;

        typedef typename Derived::Index Index;
        Index row = 0;
        Index col = 0;

        Matrix<Scalar, Dynamic, Dynamic> m;

        //unsigned int coeffSeparatorSize = fmt.coeffSeparator.size();
        //unsigned int matPrefixSize = fmt.matPrefix.size();
        //unsigned int matSuffixSize = fmt.matSuffix.size();

        size_t skip = fmt.rowSeparator.size() +
                      //fmt.rowPrefix.size() +
                      fmt.rowSuffix.size() +
                      fmt.rowSpacer.size();

        // try to find unique identifiable char's in
        // fmt.coeffSeparator, fmt. rowSeparator and fmt.matSuffix
        // so that we can detect them with s.peek()
        char coeffSeparatorChar = '\0';
        char rowSeparatorChar = '\0';
        char rowSuffixChar = '\0';
        char matSuffixChar = '\0';

        size_t t = fmt.coeffSeparator.find_first_not_of(' ');
        if (t != std::string::npos)
            coeffSeparatorChar = fmt.coeffSeparator.data()[t];

        t = fmt.rowSuffix.find_first_not_of(' ');
        if (t == std::string::npos) {
            t = fmt.rowSeparator.find_first_not_of(' ');
            if (t != std::string::npos)
                rowSeparatorChar = fmt.rowSeparator.data()[t];
        } else
            rowSeparatorChar = fmt.rowSuffix.data()[t];
        eigen_assert(rowSeparatorChar != '\0'); // we need at least one rowSep.

        t = fmt.matSuffix.find_first_not_of(' ');
        if (t != std::string::npos)
            matSuffixChar = fmt.matSuffix.data()[t];

        eigen_assert((coeffSeparatorChar != rowSeparatorChar != matSuffixChar) &&
                     "unique pre- and suffixes are needed!");

        // skip Matrix Prefix (if defined in Format):
        s.ignore(fmt.matPrefix.size());

        while (!s.eof() && s.good()) {
            if (s.peek() == matSuffixChar) {
                s.ignore(fmt.matSuffix.size());
                break;
            }

            if (s.peek() == rowSeparatorChar) {
                s.ignore(skip);
                row++;
                col = 0;
            }

            if (col == 0)
                s.ignore(fmt.rowPrefix.size());
            else if (coeffSeparatorChar != '\0')
                s.ignore(fmt.coeffSeparator.size());

            if (s.good()) {
                Scalar d;
                s >> d;
                m(row, col++) = d;
            }
        }

        return s;
    }
} // namespace Eigen

// Generic iterator interface:
template<typename Value_t, typename Container_t>
class index_iterator : public std::iterator<std::random_access_iterator_tag, Value_t>
{
protected:
   Container_t* container_;
   int          index_;

public:
   index_iterator() : container_(0), index_(0) {}

   index_iterator(Container_t& container, int index) : container_(&container), index_(index) {}

   bool operator==(const index_iterator& other) { return container_ == other.container_ && index_ == other.index_; }

   bool operator!=(const index_iterator& other) { return !(*this == other); }

   Value_t&       operator*()       { return (*container_)[index_]; }
   Value_t const& operator*() const { return (*container_)[index_]; }

   Value_t*       operator->()       { return &((*container_)[index_]); }
   Value_t const* operator->() const { return &((*container_)[index_]); }

   index_iterator& operator++()   { ++index_; return *this;}
   index_iterator operator++(int) { index_iterator prev(*this); operator++(); return prev;}

   index_iterator& operator--() { --index_; return *this;}
   index_iterator operator--(int) { index_iterator prev(*this); operator--();
return prev;}

   friend index_iterator operator+(const index_iterator& a, int b) { index_iterator ret(a); ret += b; return ret; }
   friend index_iterator operator-(const index_iterator& a, int b) { index_iterator ret(a); ret -= b; return ret; }
   friend index_iterator operator+(int a, const index_iterator& b) { index_iterator ret(b); ret += a; return ret; }
   friend index_iterator operator-(int a, const index_iterator& b) { index_iterator ret(b); ret -= a; return ret; }

   int operator-(const index_iterator& other) const { return index_ - other.index_; }

   bool operator< (const index_iterator& other) { return container_ == other.container_ && index_ <  other.index_; }
   bool operator<=(const index_iterator& other) { return container_ == other.container_ && index_ <= other.index_; }
   bool operator> (const index_iterator& other) { return container_ == other.container_ && index_ >  other.index_; }
   bool operator>=(const index_iterator& other) { return container_ == other.container_ && index_ >= other.index_; }

   index_iterator& operator+=(int b) { index_ += b; }
   index_iterator& operator-=(int b) { index_ -= b; }

   Value_t&       operator[](int i)       { return (*container_)[i]; }
   Value_t const& operator[](int i) const { return (*container_)[i]; }
};

template<typename Value_t, typename Container_t>
inline index_iterator<Value_t, Container_t> index_begin(Container_t& container)
{
   return index_iterator<Value_t, Container_t>(container, 0);
}

template<typename Value_t, typename Container_t>
inline index_iterator<Value_t, Container_t> index_end(Container_t& container)
{
   return index_iterator<Value_t, Container_t>(container, container.size());
}

template<typename Value_t, typename Container_t>
inline index_iterator<const Value_t, const Container_t> index_begin(const Container_t& container)
{
   return index_iterator<const Value_t, const Container_t>(container, 0);
}
template<typename Value_t, typename Container_t>
inline index_iterator<const Value_t, const Container_t> index_end(const Container_t& container)
{
   return index_iterator<const Value_t, const Container_t>(container, container.size());
}

#endif // UNUSED_CODE_H_
