#ifndef EIGEN_H_
#define EIGEN_H_

//#define EIGEN_IO_FORMAT_DONT_THROW_EXCEPTIONS // disables throwing IOFormatExceptions in Eigen::import_matrix()

// some usefull printing formats:
#define EIGEN_IOFMT_CSV     IOFormat(FullPrecision, 0, ",") // <-- ',' without space is important
#define EIGEN_IOFMT_PRETTY  IOFormat(4, 0, ", ", "\n", "[", "]")
#define EIGEN_IOFMT_OCTAVE  IOFormat(StreamPrecision, 0, ", ", ";\n", "", "", "[", "]")

#define EIGEN_DEFAULT_IO_FORMAT  EIGEN_IOFMT_OCTAVE // set 'pretty' as global default

#include "eigen_addons.h"
#include "eigen_io.h"
#include "eigen_iterator.h"

#include <Eigen/Dense>

using namespace Eigen;

#endif // EIGEN_H_
