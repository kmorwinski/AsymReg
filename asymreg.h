#ifndef ASYMREG_H_
#define ASYMREG_H_

#define ASYMREG_DATSRC_SIZE  11

#define ASYMREG_GRID_SIZE  200

#define ASYMREG_RECORDING_ANGLES  10

#include "eigen.h"

class BilinearInterpol;

class AsymReg {
public:
    inline static BilinearInterpol *sourceFunction()
    { return m_sourceFunc; }

    static Eigen::MatrixXd sourceFunctionPlotData(double *time);

    static void createSourceFunction(const Eigen::MatrixXd &srcDat);

private:
    static void setSourceFunction(BilinearInterpol *func);

    static BilinearInterpol *m_sourceFunc;
};

#endif // ASYMREG_H_
