#ifndef EIGEN_ITERATOR_H
#define EIGEN_ITERATOR_H

namespace Eigen {
#if 0
// support range-based for-loops (C++11):
//   (end() function works only for vectors, so we
//   only allow vector types!!!)
template<typename _Scalar, int _Rows, int _Cols>
_Scalar *begin(Matrix<_Scalar, _Rows, _Cols> &mat)
{
    EIGEN_STATIC_ASSERT((_Rows == 1) || (_Cols == 1),
                        YOU_TRIED_CALLING_A_VECTOR_METHOD_ON_A_MATRIX);
    return mat.data();
}

template<typename _Scalar, int _Rows, int _Cols, int _Options>
_Scalar *end(Matrix<_Scalar, _Rows, _Cols, _Options> &mat)
{
    EIGEN_STATIC_ASSERT((_Rows == 1) || (_Cols == 1),
                        YOU_TRIED_CALLING_A_VECTOR_METHOD_ON_A_MATRIX);

    if (_Options & RowMajor) { // RowVector
        if (_Cols == Dynamic)
            return &mat.data()[mat.cols()-1];
        return &mat.data()[_Cols-1];
    }

    // Vector:
    if (_Rows == Dynamic)
        return &mat.data()[mat.rows()-1];
    return &mat.data()[_Rows-1];
}
#endif // 0
} // namespace Eigen

#endif // EIGEN_ITERATOR_H
