#include "plottersettings.h"

#include <QtCore/QMap>
#include <QtCore/QVariant>

ContourPlotterSettings::ContourPlotterSettings()
    : PlotterSettings()
{
    m_axis = 3;

    setAxisSpan({0., 10.}, X_Axis);
    setAxisSpan({0., 10.}, Y_Axis);
    setAxisSpan({-5., 5.}, Z_Axis);

    setTitle("contour plot", 1);
}

PlotterSettings::PlotterSettings()
    : QObject(),
      m_page(Page_DIN_A4_Landscape),
      m_axisSpans({{0.,0.}, {0.,0.}, {0.,0.}}), // invalid axis spans
      m_axisTitles({"X axis", "Y axis", "Z axis"}),
      m_titles({"", "", "", ""}),
      m_imageSize(ImageStandardWidth, ImageStandardHeight),
      m_pageBorder(false),
      m_fontIndex(32), // Times Roman
      m_axis(0)
{}

int PlotterSettings::axis() const
{
    return m_axis;
}

PlotterSettings::Span PlotterSettings::axisSpan(PlotterSettings::Axis axis) const
{
    return m_axisSpans.at(axis);
}

QStringList PlotterSettings::axisTitles() const
{
    return m_axisTitles;
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

QMap<QString, QVariant> PlotterSettings::imageSizeMap() const
{
    QMap<QString, QVariant> ret;
    ret["height"] = QVariant::fromValue(m_imageSize.height());
    ret["width"] = QVariant::fromValue(m_imageSize.width());

    return ret;
}

QSize PlotterSettings::imageSize() const
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

void PlotterSettings::setAxisTitles(const QStringList &axisTitles)
{
    m_axisTitles = axisTitles;
}

void PlotterSettings::setAxisSpan(const PlotterSettings::Span &axisSpan, Axis axis)
{
    m_axisSpans[axis] = axisSpan;
}

void PlotterSettings::setFontIndex(int fontIndex)
{
    m_fontIndex = fontIndex;
}

void PlotterSettings::setImageSizeMap(const QMap<QString, QVariant> &imageSize)
{
    QSize s = QSize(imageSize["height"].toInt(), imageSize["width"].toInt());
    m_imageSize = s;
}

void PlotterSettings::setImageSize(const QSize &imageSize)
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

void PlotterSettings::setXaxisSpan(const QMap<QString, QVariant> &axisSpan)
{
    Span s = {axisSpan["from"].toDouble(), axisSpan["to"].toDouble()};
    setAxisSpan(s, X_Axis);
}

void PlotterSettings::setYaxisSpan(const QMap<QString, QVariant> &axisSpan)
{
    Span s = {axisSpan["from"].toDouble(), axisSpan["to"].toDouble()};
    setAxisSpan(s, Y_Axis);
}

void PlotterSettings::setZaxisSpan(const QMap<QString, QVariant> &axisSpan)
{
    Span s = {axisSpan["from"].toDouble(), axisSpan["to"].toDouble()};
    setAxisSpan(s, Z_Axis);
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

QMap<QString, QVariant> PlotterSettings::xAxisSpan() const
{
    QMap<QString, QVariant> ret;

    Span span = axisSpan(X_Axis);
    ret["from"] = QVariant::fromValue(span.from);
    ret["to"] = QVariant::fromValue(span.to);

    return ret;
}

QMap<QString, QVariant> PlotterSettings::yAxisSpan() const
{
    QMap<QString, QVariant> ret;

    Span span = axisSpan(Y_Axis);
    ret["from"] = QVariant::fromValue(span.from);
    ret["to"] = QVariant::fromValue(span.to);

    return ret;
}

QMap<QString, QVariant> PlotterSettings::zAxisSpan() const
{
    QMap<QString, QVariant> ret;

    Span span = axisSpan(Z_Axis);
    ret["from"] = QVariant::fromValue(span.from);
    ret["to"] = QVariant::fromValue(span.to);

    return ret;
}

#include "plottersettings.moc"
