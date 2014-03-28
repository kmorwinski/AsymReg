#ifndef SVGVIEWER_H_
#define SVGVIEWER_H_

#include <QtGui/QMainWindow>

class SvgViewer : public QMainWindow
{
    Q_OBJECT

public:
    SvgViewer(const QString &file, const QString &title);

private slots:
    void saveImage();

private:
    QString m_file;
    bool m_saved;
};

#endif // SVGVIEWER_H_
