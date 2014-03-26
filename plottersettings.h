#ifndef PLOTTERSETTINGS_H_
#define PLOTTERSETTINGS_H_

#include <QString>
#include <QStringList>
#include <QPair>

class PlotterSettings
{
public:
    enum OutputType {
        Display_Widget,
        SVG_Image
    };

    enum {
        StandardHeight = 603, /**< @see http://www2.mps.mpg.de/dislin/kap6.html#WINSIZ */
        StandardWidth = 853, /**< @see http://www2.mps.mpg.de/dislin/kap6.html#WINSIZ */
        MaximumHeight = 904,
        MaximumWidth = 1280
    };

    enum Axis {
        X_Axis = 0,
        Y_Axis = 1,
        Z_Axis = 2
    };

    typedef QPair<double, double> Span;

    PlotterSettings();

    int axis() const;

    QString axisTitle(Axis axis) const;
    void setAxisTitle(const QString &axisTitle, Axis axis);

    QStringList axisTitles() const;
    void setAxisTitles(const QStringList &axisTitles);

    Span axisSpan(Axis axis) const;
    void setAxisSpan(const Span &axisSpan, Axis axis);

    QList<Span> axisSpans() const;
    void setAxisSpans(const QList<Span> &axisSpans);

    QString title(int n) const;
    void setTitle(const QString &title, int n);

    QStringList titles() const;
    void setTitles(const QStringList &titles);

    QPair<int, int> imageSize() const;
    void setImageSize(const QPair<int, int> &imageSize);

protected:
    int m_axis; // set by dervived class!

private:
    QStringList m_axisTitles;
    QList<QPair<double, double> > m_axisSpans;
    QStringList m_titles;
    QPair<int, int> m_imageSize;
};

class ContourPlotterSettings : public PlotterSettings
{
public:
    ContourPlotterSettings();
};

#endif // PLOTTERSETTINGS_H_
