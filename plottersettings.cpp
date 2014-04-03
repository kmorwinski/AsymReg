#include "plottersettings.h"

ContourPlotterSettings::ContourPlotterSettings()
    : PlotterSettings()
{
    m_axis = 3;
    QList<Span> s = {qMakePair(0.,10.), qMakePair(0.,10.), qMakePair(-5.,5.)};
    setAxisSpans(s);

    setTitle("contour plot", 1);
}

PlotterSettings::PlotterSettings()
    : m_page(Page_DIN_A4_Portrait),
      m_axisSpans({qMakePair(0.,0.), qMakePair(0.,0.), qMakePair(0.,0.)}), // invalid axis spans
      m_axisTitles({"X axis", "Y axis", "Z axis"}),
      m_titles({"", "", "", ""}),
      m_imageSize(ImageStandardHeight, ImageStandardWidth),
      m_pageBorder(false),
      m_fontIndex(32), // Times Roman
      m_axis(0)
{}

PlotterSettings::~PlotterSettings()
{}

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

QString PlotterSettings::font() const
{
    return fonts().at(m_fontIndex);
}

int PlotterSettings::fontIndex() const
{
    return m_fontIndex;
}

QStringList PlotterSettings::fonts()
{
    QStringList ret;
    ret << "Times-Roman"                  << "Courier"
        << "Times-Bold"                   << "Courier-Bold"
        << "Times-Italic"                 << "Courier-Oblique"
        << "Times-BoldItalic"             << "Courier-BoldOblique"
        << "Helvetica"                    << "AvantGarde-Book"
        << "Helvetica-Bold"               << "AvantGarde-Demi"
        << "Helvetica-Oblique"            << "AvantGarde-BookOblique"
        << "Helvetica-BoldOblique"        << "AvantGarde-DemiOblique"
        << "Helvetica-Narrow"             << "Bookman-Light"
        << "Helvetica-Narrow-Bold"        << "Bookman-LightItalic"
        << "Helvetica-Narrow-Oblique"     << "Bookman-Demi"
        << "Helvetica-Narrow-BoldOblique" << "Bookman-DemiItalic"
        << "NewCenturySchlbk-Roman"       << "Palatino-Roman"
        << "NewCenturySchlbk-Italic"      << "Palatino-Italic"
        << "NewCenturySchlbk-Bold"        << "Palatino-Bold"
        << "NewCenturySchlbk-BoldItalic"  << "Palatino-BoldItalic"
        << "ZapfChancery-MediumItalic"    << "Symbol"
        << "ZapfDingbats";
    ret.sort();

    return ret;
}

QPair<int, int> PlotterSettings::imageSize() const
{
    return m_imageSize;
}


PlotterSettings::PageType PlotterSettings::page() const
{
    return m_page;
}

bool PlotterSettings::pageBorder() const
{
    return m_pageBorder;
}

void PlotterSettings::setAxisTitle(const QString &axisTitle, Axis axis)
{
    m_axisTitles[axis] = axisTitle;
}

void PlotterSettings::setAxisTitles(const QStringList &axisTitles)
{
    m_axisTitles = axisTitles;
}

PlotterSettings::Span PlotterSettings::axisSpan(PlotterSettings::Axis axis) const
{
    return m_axisSpans.at(axis);
}

void PlotterSettings::setAxisSpan(const QPair<double, double> &axisSpan, Axis axis)
{
    m_axisSpans[axis] = axisSpan;
}

void PlotterSettings::setAxisSpans(const QList<Span> &axisSpans)
{
    m_axisSpans = axisSpans;
}

void PlotterSettings::setFont(const QString &font)
{
    QStringList fontList = fonts();
    int index = fontList.indexOf(font);
    if (index != -1)
        m_fontIndex = index;
}

void PlotterSettings::setFontIndex(int fontIndex)
{
    m_fontIndex = fontIndex;
}

void PlotterSettings::setImageSize(const QPair<int, int> &imageSize)
{
    m_imageSize = imageSize;
}

void PlotterSettings::setPage(PlotterSettings::PageType page)
{
    m_page = page;
}

void PlotterSettings::setPageBorder(bool border)
{
    m_pageBorder = border;
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
