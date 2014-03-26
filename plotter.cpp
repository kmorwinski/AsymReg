#include "plotter.h"

using namespace Eigen;

ContourPlotter::ContourPlotter(const PlotterSettings &settings, OutputType out)
    : Plotter(settings, out)
{
#if 0
    // AXSPOS determines the position of an axis system.
    // level 1
    // NXA, NYA	are plot coordinates that define the lower left corner of an axis system. By default, axis systems are centred in the X-direction while NYA is set to the value (page height - 300).
    m_dislin.axspos(300, 1850);

    // The routine AX3LEN defines the axis lengths of a coloured axis system.
    // level 1,2,3
    // NXL, NYL, NZL are the axis lengths in plot coordinates
    m_dislin.ax3len(2200, 1400, 1400);

    // This routine sets the number of digits after the decimal point displayed in labels.
    // level 1,2,3
    // NDIG = -2	the number of digits is automatically calculated by DISLIN.
    // NDIG = -1	defines integer labels.
    // NDIG = 0	defines integer labels followed by a decimal point.
    // NDIG = n	defines the number of digits after the decimal point. The last digit will be rounded up.
    // CAX is a character string that defines the axes. Default: (1, 'XYZ').
    m_dislin.labdig(-1, "x");

    // This routine is used to define the number of ticks between axis labels.
    // level 1,2,3
    // NTIC is the number of ticks (>= 0).
    // CAX is a character string that defines the axes. Default: (2, 'XYZ').
    m_dislin.ticks(9, "x");
    m_dislin.ticks(10, "y");
#endif // 0

    // The routine GRAF3 plots a 3-D axis system where the Z-axis is plotted as a colour bar.
    // level 1
    // XA, XE: are the lower and upper limits of the X-axis.
    // XOR, XSTP: are the first X-axis label and the step between labels.
    // YA, YE: are the lower and upper limits of the Y-axis.
    // YOR, YSTP: are the first Y-axis label and the step between labels.
    // ZA, ZE: are the lower and upper limits of the Z-axis.
    // ZOR, ZSTP: are the first Z-axis label and the step between labels.
    auto spans = m_settings.axisSpans();
    m_dislin.graf3(spans.at(PlotterSettings::X_Axis).first, spans.at(PlotterSettings::X_Axis).second,
                   spans.at(PlotterSettings::X_Axis).first, 1.,
                   spans.at(PlotterSettings::Y_Axis).first, spans.at(PlotterSettings::Y_Axis).second,
                   spans.at(PlotterSettings::Y_Axis).first, 1.,
                   spans.at(PlotterSettings::Z_Axis).first, spans.at(PlotterSettings::Z_Axis).second,
                   spans.at(PlotterSettings::Z_Axis).first, 1.);
}

void ContourPlotter::setData(const Eigen::VectorXd &xVec, const Eigen::VectorXd &yVec, const Eigen::MatrixXd &zMat)
{
    // CRVMAT plots a coloured surface according to a matrix
    // level 3
    // ZMAT: is a matrix of the dimension (IXDIM, IYDIM) containing Z-coordinates. The coordinates correspond to a linear grid that overlays the axis system. If XA, XE, YA and YE are the axis limits in GRAF3 or values defined with the routine SURSZE, the relationship between the grid points and the matrix elements can be described by the formula:
    // ZMAT(I,J) = F(X,Y) where X = XA + (I - 1) * (XE - XA) / (IXDIM - 1) , I = 1,..,IXDIM and
    // Y = YA + (J - 1) * (YE - YA) / (IYDIM - 1) , J = 1,..,IYDIM.
    // IXDIM, IYDIM: define the dimension of ZMAT (>= 2).
    // IXPTS, IYPTS: are the number of interpolation steps between grid lines (>= 1). CRVMAT can interpolate points linearly.
    //m_dislin.crvmat(dataMatrix, steps, steps, 1, 1);

    //surfce (const float *xray, int ixdim, const float *yray, int iydim, const float *zmat)
    m_dislin.surfce(xVec.data(), xVec.size(), yVec.data(), yVec.size(), zMat.data());
}

Plotter::Plotter(const PlotterSettings &settings, OutputType out)
    : m_settings(settings),
      m_plotted(false)
{
    setOutput(out);
    setSize(m_settings.imageSize().first, m_settings.imageSize().second);

    m_dislin.scrmod("revers");
    m_dislin.disini(); // initialiaze
    m_dislin.hwfont(); // select hardware font
    m_dislin.pagera(); // PAGERA plots a border around the page.

    // set axis titles:
    auto axisTitles = m_settings.axisTitles();
    auto axis = m_settings.axis();

    if ((1 <= axis) && !axisTitles.at(PlotterSettings::X_Axis).isEmpty())
        m_dislin.name(axisTitles.at(PlotterSettings::X_Axis).toAscii().data(), "x");

    if ((2 <= axis) && !axisTitles.at(PlotterSettings::Y_Axis).isEmpty())
        m_dislin.name(axisTitles.at(PlotterSettings::Y_Axis).toAscii().data(), "y");

    if ((3 <= axis) && !axisTitles.at(PlotterSettings::Z_Axis).isEmpty())
        m_dislin.name(axisTitles.at(PlotterSettings::Z_Axis).toAscii().data(), "z");

    // set titles:
    auto titles = m_settings.titles();
    auto it = titles.begin();
    int t = 0;
    do {
        t++;
        if (!it->isEmpty())
            m_dislin.titlin(it->toAscii().data(), t);
    } while (++it != titles.end());
}

Plotter::~Plotter()
{
    if (!m_plotted)
        plot();
}

void Plotter::plot()
{
    m_dislin.title(); // plot titles
    m_dislin.disfin(); // finish plot

    m_plotted = true;
}

void Plotter::setSize(int height, int width)
{
    switch (m_outputType) {
    case Display_Widget:
        m_dislin.window(100, 100, width, height);
        break;
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

