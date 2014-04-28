#ifndef EIGEN_H_
#define EIGEN_H_

//#define EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS // disables throwing IOFormatExceptions in Eigen::import_matrix()

#include "eigen_addons.h"
#include "eigen_io.h"
#include "eigen_iterator.h"

#include <Eigen/Dense>

using namespace Eigen;

#define EIGEN_FMT_CSV    IOFormat(FullPrecision, 0, ",") // <-- ',' without space is important
#define EIGEN_FMT_PRETTY IOFormat(4, 0, ", ", "\n", "[", "]")

#endif // EIGEN_H_
