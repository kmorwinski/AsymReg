#ifndef PLOTTER_H_
#define PLOTTER_H_

#include <discpp.h>

#include "eigen.h"
#include "plottersettings.h"

class Plotter {
public:
    enum OutputType {
        Display_Widget,
        SVG_Image
    };

    typedef std::tuple<double, double> Span;

    Plotter(const PlotterSettings &settings, OutputType = SVG_Image);
    ~Plotter();

    void plot();

protected:
    Dislin m_dislin;
    PlotterSettings m_settings;

private:
    void setOutput(OutputType out);
    void setSize(int height, int width);

    OutputType m_outputType;
    bool m_plotted;
};

class ContourPlotter : public Plotter
{
public:
    ContourPlotter(const PlotterSettings &settings, OutputType out = SVG_Image);

    void setData(const Eigen::MatrixXd &zMat);
    void setData(const double *zmat, int xSteps, int ySteps);
};

#endif // PLOTTER_H_
