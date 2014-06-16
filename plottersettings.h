#ifndef PLOTTERSETTINGS_H_
#define PLOTTERSETTINGS_H_

#include <array>
#include <string>

class PlotterSettings;

void copySettings(const PlotterSettings *from, PlotterSettings *to); // replaces copy-c'tor and assignment operator

#ifdef QT_CORE_LIB
#  include <QtCore/QString>
#  include <QtCore/QStringList>

class PlotterSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(PageType Page READ page WRITE setPage)
    Q_PROPERTY(QMap X_AxisSpan READ xAxisSpan WRITE setXaxisSpan)
    Q_PROPERTY(QMap Y_AxisSpan READ yAxisSpan WRITE setYaxisSpan)
    Q_PROPERTY(QMap Z_AxisSpan READ zAxisSpan WRITE setZaxisSpan)
    Q_PROPERTY(QStringList AxisTitles READ qAxisTitles WRITE qSetAxisTitles)
    Q_PROPERTY(QStringList Titles READ qTitles WRITE qSetTitles)
    Q_PROPERTY(QMap ImageSize READ imageSizeMap WRITE setImageSizeMap)
    Q_PROPERTY(bool PageBorder READ pageBorder WRITE setPageBorder)
    Q_PROPERTY(int Font READ fontIndex WRITE setFontIndex)

    Q_ENUMS(PageType)
#else // QT_CORE_LIB
class PlotterSettings
{
#endif // (else) QT_CORE_LIB
public:
    enum PageType {
        Page_DIN_A4_Landscape,
        Page_DIN_A4_Portrait
    };

    enum ImageDimension {
        ImageStandardHeight = 603, /**< @see http://www2.mps.mpg.de/dislin/kap6.html#WINSIZ */
        ImageStandardWidth = 853, /**< @see http://www2.mps.mpg.de/dislin/kap6.html#WINSIZ */
        ImageMaximumHeight = 904,
        ImageMaximumWidth = 1280
    };

    enum Axis {
        X_Axis = 0,
        Y_Axis = 1,
        Z_Axis = 2
    };

    struct Span {
        double from;
        double to;
        friend inline bool operator==(const Span &s1, const Span &s2);
    };

    /**
     * A simple replacement class for QSize.
     */
    class Size {
    public:
        Size(int width, int height) : wd(width), ht(height)
        {}

        inline int width() const
        { return wd; }
        inline int height() const
        { return ht; }

        friend inline bool operator==(const Size &s1, const Size &s2);

    private:
        int wd;
        int ht;
    };

    PlotterSettings();

    int axis() const;

    std::array<std::string, 3> axisTitles() const;
    void setAxisTitles(const std::array<std::string, 3> &axisTitles);

    Span axisSpan(Axis axis) const;
    void setAxisSpan(const Span &axisSpan, Axis axis);

    std::string font() const;
    int fontIndex() const;
    void setFontIndex(int fontIndex);

    Size imageSize() const;
    void setImageSize(const Size &imageSize);

    std::string title(int n) const;
    void setTitle(const std::string &title, int n);

    std::array<std::string, 4> titles() const;
    void setTitles(const std::array<std::string, 4> &titles);

    PageType page() const;
    void setPage(PageType page);

    bool pageBorder() const;
    void setPageBorder(bool border);

#ifdef QT_CORE_LIB
    // only for QJson/Q_PROPERTY:
    QMap<QString, QVariant> imageSizeMap() const;
    void setImageSizeMap(const QMap<QString, QVariant> &imageSizeMap);

    QMap<QString, QVariant> xAxisSpan() const;
    void setXaxisSpan(const QMap<QString, QVariant> &axisSpan);

    QMap<QString, QVariant> yAxisSpan() const;
    void setYaxisSpan(const QMap<QString, QVariant> &axisSpan);

    QMap<QString, QVariant> zAxisSpan() const;
    void setZaxisSpan(const QMap<QString, QVariant>  &axisSpan);

    // some additional Qt implementations:
    static QStringList fonts();

    QStringList qAxisTitles() const;
    void qSetAxisTitles(const QStringList &qAxisTitles);

    QString qTitle(int n) const;
    void qSetTitle(const QString &qTitle, int n);

    QStringList qTitles() const;
    void qSetTitles(const QStringList &titles);
#endif // QT_CORE_LIB

    friend void copySettings(const PlotterSettings *from, PlotterSettings *to);

protected:
    int m_axis; // set by dervived class!

private:
    PlotterSettings(const PlotterSettings &);            // disable copy c'tor and assignment operator,
    PlotterSettings &operator=(const PlotterSettings &); // use copySettings() from above

    std::array<Span, 3> m_axisSpans;
    std::array<std::string, 3> m_axisTitles;
    std::array<std::string, 4> m_titles;
    PageType m_page;
    Size m_imageSize;

    bool m_pageBorder;
    int m_fontIndex;

#ifdef QT_CORE_LIB
signals:
    void settingsChanged();
#endif
};

class ContourPlotterSettings : public PlotterSettings
{
public:
    ContourPlotterSettings();
};

#endif // PLOTTERSETTINGS_H_
