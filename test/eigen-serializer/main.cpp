#include <iostream>
#include <fstream>

#define WRITE // comment for reading

//#define IOFMT_COMMAINIT
//#define IOFMT_CLEAN
#define IOFMT_CSV
//#define IOFMT_HEAVY
//#define IOFMT_OCTAVE

#if defined IOFMT_COMMAINIT
#  define EIGEN_DEFAULT_IO_FORMAT IOFormat(StreamPrecision, DontAlignCols, ", ", ", ", "", "", " << ", ";")
#  define FILE "data_commainit.txt"
#elif defined IOFMT_CLEAN
#  define EIGEN_DEFAULT_IO_FORMAT IOFormat(4, 0, ", ", "\n", "[", "]")
#  define FILE "data_clean.txt"
#elif defined IOFMT_CSV
#  define EIGEN_DEFAULT_IO_FORMAT IOFormat(FullPrecision, 0, ",")
//#  define EIGEN_DEFAULT_IO_FORMAT IOFormat(FullPrecision, DontAlignCols, ",\t")
#  define FILE "data.csv"
#elif defined IOFMT_HEAVY
#  define EIGEN_DEFAULT_IO_FORMAT IOFormat(FullPrecision, 0, ", ", ";\n", "[", "]", "[", "]")
#  define FILE "data_heavy.txt"
#elif defined IOFMT_OCTAVE
#  define EIGEN_DEFAULT_IO_FORMAT IOFormat(StreamPrecision, 0, ", ", ";\n", "", "", "[", "]")
#  define FILE "data_octave.txt"
#else
#  define FILE "data.txt"
#endif

#include "../../eigen.h"

using namespace std;
using namespace Eigen;

int main()
{
    Matrix3d m;

    fstream fs;
#ifdef WRITE
    fs.open(FILE, fstream::out | fstream::trunc);
#else
    fs.open(FILE, fstream::in);
#endif
    cout << "File \"" << FILE << "\" opened with status: " << fs.is_open() << std::endl;

#ifdef WRITE
    m << 1.111111, 2, 3.33333, 4, 5, 6, 7, 8.888888, 9;
    cout << "Writing Matrix:" << endl;
    fs << m;
    cout << m << endl;
    cout << "done!" << endl;
#else
    cout << "Reading Matrix:" << endl;
    fs >> m;
    cout << m << endl;
    cout << "done!" << endl;
#endif

    fs.close();
    return 0;
}

