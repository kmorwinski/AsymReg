#ifndef PLOTTER_H_
#define PLOTTER_H_

// Standard Library Header:
#include <tuple>

// other:
#include <discpp.h>

class BilinearInterpol;

class Plotter {
public:
    enum OutputType {
        Display_Widget,
        SVG_Image
    };

    enum {
        StandardHeight = 603, /**< @see http://www2.mps.mpg.de/dislin/kap6.html#WINSIZ */
        StandardWidth = 853 /**< @see http://www2.mps.mpg.de/dislin/kap6.html#WINSIZ */
    };

    typedef std::tuple<double, double> Span;

    Plotter(float sizeScale = 1., OutputType out = SVG_Image);
    Plotter(int height = StandardHeight,int width = StandardWidth, OutputType out = SVG_Image);
    ~Plotter();

    void setTitle(std::string str, int row);


    void plot();
    void setSize(float sizeScale);
    void setSize(int height, int width);

protected:
    Dislin m_dislin;

private:
    void setOutput(OutputType out);

    OutputType m_outputType;
    bool m_plotted;
};

class ContourPlotter : public Plotter
{
public:
    ContourPlotter(float sizeScale = 1., OutputType out = SVG_Image);
    ContourPlotter(int height = StandardHeight,int width = StandardWidth, OutputType out = SVG_Image);

    void plotFunction(const BilinearInterpol &func, Span xSpan, Span ySpan, int steps);
    void plotData(float *dataMatrix, Span xSpan, Span ySpan, Span zSpan, int steps);

private:
    float *m_data;
};

#endif // PLOTTER_H_
