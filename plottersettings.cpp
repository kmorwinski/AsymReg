#include "plottersettings.h"

ContourPlotterSettings::ContourPlotterSettings()
    //: PlotterSettings()
{
    m_axis = 3;

    setTitle("contour plot", 1);
}

PlotterSettings::PlotterSettings()
    : m_axisSpans({qMakePair(0.,0.), qMakePair(0.,0.), qMakePair(0.,0.)}), // invalid axis spans
      m_axisTitles({"X axis", "Y axis", "Z axis"}),
      m_imageSize(StandardHeight, StandardWidth),
      m_titles({"", "", "", ""}),
      m_axis(-1)
{
}

int PlotterSettings::axis() const
{
    return m_axis;
}

QString PlotterSettings::axisTitle(Axis axis) const
{
    return m_axisTitles.at(axis);
}

QStringList PlotterSettings::axisTitles() const
{
    return m_axisTitles;
}

QList<PlotterSettings::Span> PlotterSettings::axisSpans() const
{
    return m_axisSpans;
}

QPair<int, int> PlotterSettings::imageSize() const
{
    return m_imageSize;
}

void PlotterSettings::setAxisTitle(const QString &axisTitle, Axis axis)
{
    m_axisTitles[axis] = axisTitle;
}

void PlotterSettings::setAxisTitles(const QStringList &axisTitles)
{
    m_axisTitles = axisTitles;
}

void PlotterSettings::setAxisSpan(const QPair<double, double> &axisSpan, Axis axis)
{
    m_axisSpans[axis] = axisSpan;
}

void PlotterSettings::setAxisSpans(const QList<Span> &axisSpans)
{
    m_axisSpans = axisSpans;
}

void PlotterSettings::setImageSize(const QPair<int, int> &imageSize)
{
    m_imageSize = imageSize;
}

void PlotterSettings::setTitle(const QString &title, int n)
{
    Q_ASSERT(n <= 4);

    m_titles[n-1] = title;
}

void PlotterSettings::setTitles(const QStringList &titles)
{
    m_titles = titles;
}

QString PlotterSettings::title(int n) const
{
    Q_ASSERT(n <= 4);

    return m_titles.at(n-1);
}

QStringList PlotterSettings::titles() const
{
    return m_titles;
}
