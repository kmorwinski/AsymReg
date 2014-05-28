#ifndef ASYMREG_H_
#define ASYMREG_H_

#define ASYMREG_DATSRC_SIZE  10

#define ASYMREG_GRID_SIZE  200

#define AR_NUM_REC_ANGL  4

#define AR_TRGT_SMPL_RATE  0.5

#include "eigen.h"

class Duration;
class BilinearInterpol;

class AsymReg {
public:
    inline static BilinearInterpol *sourceFunction()
    { return m_sourceFunc; }

    static MatrixXd sourceFunctionPlotData(Duration *time = nullptr);

    static void createSourceFunction(const MatrixXd &srcDat);

    static void generateDataSet(Duration *time = nullptr);

    static Matrix<double, Dynamic, Dynamic> &regularize(Duration *time = nullptr);

private:
    static void setSourceFunction(BilinearInterpol *func);

    static BilinearInterpol *m_sourceFunc;
    static RowVectorXd m_DataSet[AR_NUM_REC_ANGL];
    static Matrix<double, Dynamic, Dynamic> m_Result;
};

#endif // ASYMREG_H_
