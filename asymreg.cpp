#include "asymreg.h"

#include "eigen.h"
#include "interpol.h"

BilinearInterpol *AsymReg::m_sourceFunc(nullptr);

void AsymReg::setSourceFunction(BilinearInterpol *func)
{
    assert(func != nullptr);

    delete m_sourceFunc;
    m_sourceFunc = func;
}

