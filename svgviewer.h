#ifndef SVGVIEWER_H_
#define SVGVIEWER_H_

#include <QtGui/QMainWindow>

class SvgViewer : public QMainWindow
{
    Q_OBJECT

public:
    SvgViewer(const QString &file, const QString &title);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    bool saveImage();

private:
    QString readableFileSize(qint64 fileSize) const;

    QString m_file;
    QString m_title;
};

#endif // SVGVIEWER_H_
