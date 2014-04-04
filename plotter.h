#ifndef PLOTTER_H_
#define PLOTTER_H_

#include <discpp.h>

#include "eigen.h"

class PlotterSettings;

class Plotter {
public:
    enum OutputType {
        Output_Display_Widget,
        Output_SVG_Image
    };

    Plotter(const PlotterSettings *settings, OutputType = Output_SVG_Image);
    ~Plotter();

    void plot();

protected:
    Dislin m_dislin;
    const PlotterSettings *m_settings;

private:
    void setFont();
    void setOutput();
    void setPage();
    void setSize();

    OutputType m_outputType;
    bool m_plotted;
};

class ContourPlotter : public Plotter
{
public:
    ContourPlotter(const PlotterSettings *settings, OutputType out = Output_SVG_Image);

    void setData(const Eigen::MatrixXd &zMat);
    void setData(const double *zmat, int xSteps, int ySteps);
};

#endif // PLOTTER_H_
