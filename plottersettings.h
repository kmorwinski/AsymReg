#ifndef PLOTTERSETTINGS_H_
#define PLOTTERSETTINGS_H_

#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/QStringList>

class PlotterSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(PageType page READ page WRITE setPage)
    Q_PROPERTY(QMap xAxisSpan READ xAxisSpan WRITE setXaxisSpan)
    Q_PROPERTY(QMap yAxisSpan READ yAxisSpan WRITE setYaxisSpan)
    Q_PROPERTY(QMap zAxisSpan READ zAxisSpan WRITE setZaxisSpan)
    Q_PROPERTY(QStringList axisTitles READ axisTitles WRITE setAxisTitles)
    Q_PROPERTY(QStringList titles READ titles WRITE setTitles)
    Q_PROPERTY(QMap imageSize READ imageSizeMap WRITE setImageSizeMap)
    Q_PROPERTY(bool pageBorder READ pageBorder WRITE setPageBorder)
    Q_PROPERTY(int font READ fontIndex WRITE setFontIndex)

    Q_ENUMS(PageType)

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

    PlotterSettings();

    int axis() const;

    QStringList axisTitles() const;
    void setAxisTitles(const QStringList &axisTitles);

    Span axisSpan(Axis axis) const;
    void setAxisSpan(const Span &axisSpan, Axis axis);

    QString font() const;
    static QStringList fonts();

    int fontIndex() const;
    void setFontIndex(int fontIndex);

    QSize imageSize() const;
    void setImageSize(const QSize &imageSize);

    QString title(int n) const;
    void setTitle(const QString &title, int n);

    QStringList titles() const;
    void setTitles(const QStringList &titles);

    PageType page() const;
    void setPage(PageType page);

    bool pageBorder() const;
    void setPageBorder(bool border);

    // only for QJson/Q_PROPERTY:
    QMap<QString, QVariant> imageSizeMap() const;
    void setImageSizeMap(const QMap<QString, QVariant> &imageSizeMap);

    QMap<QString, QVariant> xAxisSpan() const;
    void setXaxisSpan(const QMap<QString, QVariant> &axisSpan);

    QMap<QString, QVariant> yAxisSpan() const;
    void setYaxisSpan(const QMap<QString, QVariant> &axisSpan);

    QMap<QString, QVariant> zAxisSpan() const;
    void setZaxisSpan(const QMap<QString, QVariant>  &axisSpan);

protected:
    int m_axis; // set by dervived class!

private:
    PageType m_page;
    QList<Span> m_axisSpans;
    QStringList m_axisTitles;
    QStringList m_titles;
    QSize m_imageSize;

    bool m_pageBorder;
    int m_fontIndex;

signals:
    void settingsChanged();
};

class ContourPlotterSettings : public PlotterSettings
{
public:
    ContourPlotterSettings();
};

#endif // PLOTTERSETTINGS_H_
