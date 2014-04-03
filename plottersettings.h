#ifndef PLOTTERSETTINGS_H_
#define PLOTTERSETTINGS_H_

#include <QString>
#include <QStringList>
#include <QPair>

class PlotterSettings
{
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

    typedef QPair<double, double> Span;

    PlotterSettings();
    virtual ~PlotterSettings();

    int axis() const;

    QString axisTitle(Axis axis) const;
    void setAxisTitle(const QString &axisTitle, Axis axis);

    QStringList axisTitles() const;
    void setAxisTitles(const QStringList &axisTitles);

    Span axisSpan(Axis axis) const;
    void setAxisSpan(const Span &axisSpan, Axis axis);

    QList<Span> axisSpans() const;
    void setAxisSpans(const QList<Span> &axisSpans);

    QString font() const;
    int fontIndex() const;
    static QStringList fonts();
    void setFont(const QString &font);
    void setFontIndex(int fontIndex);

    QPair<int, int> imageSize() const;
    void setImageSize(const QPair<int, int> &imageSize);

    QString title(int n) const;
    void setTitle(const QString &title, int n);

    QStringList titles() const;
    void setTitles(const QStringList &titles);

    PageType page() const;
    void setPage(PageType page);

    bool pageBorder() const;
    void setPageBorder(bool border);

protected:
    int m_axis; // set by dervived class!

private:
    PageType m_page;
    QList<QPair<double, double> > m_axisSpans;
    QStringList m_axisTitles;
    QStringList m_titles;
    QPair<int, int> m_imageSize;

    bool m_pageBorder;
    int m_fontIndex;
};

class ContourPlotterSettings : public PlotterSettings
{
public:
    ContourPlotterSettings();
};

#endif // PLOTTERSETTINGS_H_
