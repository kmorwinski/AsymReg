#include "plotter.h"

#include <algorithm>

#include "interpol.h"

using namespace std;

ContourPlotter::ContourPlotter(float sizeScale, OutputType out)
    : Plotter(sizeScale, out)
{
    m_dislin.axspos(300, 1850);
    m_dislin.ax3len(2200, 1400, 1400);

    m_dislin.name("X-Achse", "x");
    m_dislin.name("Y-Achse", "y");
    m_dislin.name("Z-Achse", "z");

    m_dislin.labdig(-1, "x");
    m_dislin.ticks(9, "x");
    m_dislin.ticks(10, "y");

}

ContourPlotter::ContourPlotter(int height, int width, OutputType out)
    : Plotter(height, width, out)
{
    m_dislin.axspos(300, 1850);
    m_dislin.ax3len(2200, 1400, 1400);

    m_dislin.name("X-Achse", "x");
    m_dislin.name("Y-Achse", "y");
    m_dislin.name("Z-Achse", "z");

    m_dislin.labdig(-1, "x");
    m_dislin.ticks(9, "x");
    m_dislin.ticks(10, "y");
}

void ContourPlotter::plotData(float *dataMatrix, Span xSpan, Span ySpan, Span zSpan, int steps)
{
    double left = get<0>(xSpan);
    double right = get<1>(xSpan);
    double bottom = get<0>(ySpan);
    double top = get<1>(ySpan);
    double min = get<0>(zSpan);
    double max = get<1>(zSpan);

    m_dislin.graf3(left, right, 0.0, 1.0, bottom, top, 0.0, 1.0, min, max, -2.0, 1.0);
    m_dislin.crvmat(dataMatrix, steps, steps, 1, 1);
}

Plotter::Plotter(float sizeScale, OutputType out)
    : m_plotted(false)
{
    setOutput(out);
    setSize(sizeScale);

    m_dislin.scrmod("revers");
    m_dislin.disini();
    m_dislin.pagera();
    m_dislin.complx();
}

Plotter::Plotter(int height, int width, OutputType out)
    : m_plotted(false)
{
    setOutput(out);
    setSize(height, width);

    m_dislin.scrmod("revers");
    m_dislin.disini();
    m_dislin.pagera();
    m_dislin.complx();
}

Plotter::~Plotter()
{
    if (!m_plotted)
        plot();
}

void Plotter::plot()
{
    m_dislin.title();
    m_dislin.disfin();

    m_plotted = true;
}

void Plotter::setSize(float sizeScale)
{
    int h = sizeScale * StandardHeight;
    int w = sizeScale * StandardWidth;
    setSize(h, w);
}

void Plotter::setSize(int height, int width)
{
    switch (m_outputType) {
    //case Display_Widget:
    //    m_dislin.window(100, 100, width, height);
    //    break;
    default:
        m_dislin.winsiz(width, height);
        break;
    }
}

void Plotter::setOutput(OutputType out)
{
    switch (out) {
    case SVG_Image:
        m_dislin.metafl("SVG");
        break;
    case Display_Widget:
        m_dislin.metafl("CONS");
    default:
        break;
    }

    m_outputType = out;
}

#if 0
void Plotter::plotPressureFunction(const BilinearInterpol &pressureFunction, PlotSpecs plotSpecs)
{
    // interpolation:
    double from = get<0>(plotSpecs);
    double to = get<1>(plotSpecs);
    int steps = get<2>(plotSpecs);

    double stepSize = abs(to - from) / stepSize;
    float zmat[steps][steps];

    for (int i = 0; i < steps; i++) {
        double x = i * stepSize;
        for (int j = 0; j < steps; ++j) {
            double y = j * stepSize;
            zmat[i][j] = pressureFunction.interpol(x, y);
        }
    }

    // plotting:
    Dislin dislin;
    dislin.metafl("cons");
    dislin.scrmod("revers");
    dislin.disini();
    dislin.pagera();
    dislin.complx();
    //g.axspos(450, 1800);
    //g.axslen(2200, 1200);
    dislin.axspos(300, 1850);
    dislin.ax3len(2200, 1400, 1400);

    dislin.name("X-Achse", "x");
    dislin.name("Y-Achse", "y");
    dislin.name("Z-Achse", "z");

    dislin.labdig(-1, "x");
    dislin.ticks(9, "x");
    dislin.ticks(10, "y");

    dislin.titlin("Demonstration der Klasse BilinearInterpol", 1);

    dislin.graf3(0.0, 10.0, 0.0, 1.0, 0.0, 10.0, 0.0, 1.0, -2.0, 2.0, -2.0, 1.0);
    dislin.crvmat((double *)zheight, n, n, 1, 1);

    dislin.height(50);
    dislin.title();
    dislin.disfin();

#if 0
    g.titlin("Demonstration of LinearInterpol", 1);

    int ic = g.intrgb(0.95,0.95,0.95);
    g.axsbgd(ic);

    g.graf(0.0, 10.0, 1.0, 1.0, -5.0, 5.0, -5.0, 1.0);
    g.setrgb(0.7, 0.7, 0.7);
    g.grid(1, 1);

    g.color("fore");
    g.height(50);
    g.title();

    g.color("red");
    g.curve(xray, yray, n);
#endif // 0
}
#endif // 0

