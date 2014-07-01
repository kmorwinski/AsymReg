#ifndef ASYMREG_H_
#define ASYMREG_H_

#define ASYMREG_DATSRC_SIZE  10

#define ASYMREG_GRID_SIZE  200

#define AR_NUM_REC_ANGL  50

#define AR_TRGT_SMPL_RATE  0.025

#include "eigen.h"

class BilinearInterpol;
class Duration;
class PlotterSettings;

class AsymReg {
public:
    inline static BilinearInterpol *sourceFunction()
    { return m_sourceFunc; }

    static MatrixXd sourceFunctionPlotData(Duration *time = nullptr);

    static void createSourceFunction(const MatrixXd &srcDat);

    static void generateDataSet(int recordingAngles,
                                Duration *time = nullptr);

    enum ODE_Solver {
        Euler,
        Midpoint,  // RK2
        RungeKutta // RK4
    };

    static double regularize(int recordingAngles,
                             ODE_Solver solver, int iterations, double step,
                             const PlotterSettings *pl, Duration *time = nullptr);

    static const MatrixXd &result()
    { return m_Result; }

private:
    static void setSourceFunction(BilinearInterpol *func);

    static BilinearInterpol *m_sourceFunc;
    static RowVectorXd m_DataSet[AR_NUM_REC_ANGL];
    static Matrix<double, Dynamic, Dynamic> m_Result;
};

#endif // ASYMREG_H_
