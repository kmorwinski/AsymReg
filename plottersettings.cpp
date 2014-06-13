#include "plottersettings.h"

#include <cassert>

#ifdef QT_CORE_LIB
#  include <QtCore/QMap>
#  include <QtCore/QVariant>

#  define EMIT_QTSIGNAL(SIG)  emit SIG()
#else
#  define EMIT_QTSIGNAL(SIG)
#endif

inline bool operator==(const PlotterSettings::Span &s1, const PlotterSettings::Span &s2)
{
    return (s1.from == s2.from) && (s1.to == s2.to);
}

inline bool operator==(const PlotterSettings::Size &s1, const PlotterSettings::Size &s2)
{
    return (s1.wd == s2.wd) && (s1.ht == s2.ht);
}

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
#ifdef QT_CORE_LIB
    : QObject(),
#else
    :
#endif
      m_page(Page_DIN_A4_Landscape),
      m_axisSpans({{ {0.,0.}, {0.,0.}, {0.,0.} }}), // invalid axis spans, c'tor needs 2 braces {{ }}
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

std::array<std::string, 3> PlotterSettings::axisTitles() const
{
    return m_axisTitles;
}

std::string PlotterSettings::font() const
{
#ifdef QT_CORE_LIB
    return fonts().at(m_fontIndex).toStdString();
#else
    return "Times Roman";
#endif
}

int PlotterSettings::fontIndex() const
{
    return m_fontIndex;
}

#ifdef QT_CORE_LIB
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
#endif // QT_CORE_LIB

PlotterSettings::Size PlotterSettings::imageSize() const
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

#ifdef QT_CORE_LIB
QStringList PlotterSettings::qAxisTitles() const
{
    QStringList ret;
    for (int i = 0; i < m_axis; ++i)
        ret << QString::fromStdString(m_axisTitles.at(i));

    return ret;
}

QString PlotterSettings::qTitle(int n) const
{
    Q_ASSERT(n >= 1 && n <= 4);

    return QString::fromStdString(m_titles.at(n-1));
}

QStringList PlotterSettings::qTitles() const
{
    QStringList ret;
    for (int i = 0; i < 4; ++i)
        ret << QString::fromStdString(m_titles.at(i));

    return ret;
}

void PlotterSettings::qSetAxisTitles(const QStringList &axisTitles)
{
    Q_ASSERT(axisTitles.size() <= 3);
    std::array<std::string, 3> arr;

    for (int i = 0; i < axisTitles.size(); ++i)
        arr[i] = axisTitles.at(i).toStdString();

    setAxisTitles(arr);
}

void PlotterSettings::qSetTitle(const QString &title, int n)
{
    Q_ASSERT(n >= 1 && n <= 4);

    if (m_titles.at(n-1) == title.toStdString())
        return;

    m_titles[n-1] = title.toStdString();
    EMIT_QTSIGNAL(settingsChanged);
}

void PlotterSettings::qSetTitles(const QStringList &titles)
{
    Q_ASSERT(titles.size() <= 4);

    std::array<std::string, 4> arr;

    for (int i = 0; i < titles.size(); ++i)
        arr[i] = titles.at(i).toStdString();

    setTitles(arr);
}
#endif // QT_CORE_LIB

void PlotterSettings::setAxisTitles(const std::array<std::string, 3> &axisTitles)
{
    if (m_axisTitles == axisTitles)
        return;

    m_axisTitles = axisTitles;
    EMIT_QTSIGNAL(settingsChanged);
}

void PlotterSettings::setAxisSpan(const PlotterSettings::Span &axisSpan, Axis axis)
{
    if (m_axisSpans.at(axis) == axisSpan)
        return;

    m_axisSpans[axis] = axisSpan;
    EMIT_QTSIGNAL(settingsChanged);
}

void PlotterSettings::setFontIndex(int fontIndex)
{
    if (m_fontIndex == fontIndex)
        return;

    m_fontIndex = fontIndex;
    EMIT_QTSIGNAL(settingsChanged);
}

#ifdef QT_CORE_LIB
void PlotterSettings::setImageSizeMap(const QMap<QString, QVariant> &imageSizeMap)
{
    Size size = Size(imageSizeMap["width"].toInt(), imageSizeMap["height"].toInt());
    if (m_imageSize == size)
        return;

    m_imageSize = size;
    EMIT_QTSIGNAL(settingsChanged);
}
#endif

void PlotterSettings::setImageSize(const PlotterSettings::Size &imageSize)
{
    if (m_imageSize == imageSize)
        return;

    m_imageSize = imageSize;
    EMIT_QTSIGNAL(settingsChanged);
}

void PlotterSettings::setPage(PlotterSettings::PageType page)
{
    if (m_page == page)
        return;

    m_page = page;
    EMIT_QTSIGNAL(settingsChanged);
}

void PlotterSettings::setPageBorder(bool border)
{
    if (m_pageBorder == border)
        return;

    m_pageBorder = border;
    EMIT_QTSIGNAL(settingsChanged);
}

void PlotterSettings::setTitle(const std::string &title, int n)
{
    assert(n >= 1 && n <= 4);

    if (title == m_titles.at(n-1))
        return;

    m_titles[n-1] = title;
    EMIT_QTSIGNAL(settingsChanged);
}

void PlotterSettings::setTitles(const std::array<std::string, 4> &titles)
{
    if (m_titles == titles)
        return;

    m_titles = titles;
    EMIT_QTSIGNAL(settingsChanged);
}

#ifdef QT_CORE_LIB
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
#endif // QT_CORE_LIB

std::string PlotterSettings::title(int n) const
{
    assert(n >= 1 && n <= 4);

    return m_titles.at(n-1);
}

std::array<std::string, 4> PlotterSettings::titles() const
{
    return m_titles;
}

#ifdef QT_CORE_LIB
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
#  include "plottersettings.moc"
#endif // QT_CORE_LIB
