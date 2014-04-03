#include "svgviewer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QScopedPointer>
#include <QtCore/QTextStream>

#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QShortcut>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>

#include <QtSvg/QSvgWidget>

#ifdef QT_HAS_IOCOMPRESSOR
#  include <QtSolutions/QtIOCompressor>
#endif

SvgViewer::SvgViewer(const QString &file, const QString &title)
    : m_title(title)
{
    // window title:
    static int _num = 0; // running number
    QString appName = qApp->applicationName();
    setWindowTitle(tr("Plot %1: %2[*] - %3").arg(++_num).arg(title).arg(appName));

    // toolbar:
    QAction *closeAction = new QAction(this);
    closeAction->setText(tr("&Close"));
    closeAction->setIcon(QIcon::fromTheme("window-close"));
    closeAction->setShortcut(Qt::Key_Escape);
    connect(closeAction, SIGNAL(triggered()),
            this, SLOT(close()));

    QAction *saveAsAction = new QAction(this);
    saveAsAction->setText(tr("&Save As"));
    saveAsAction->setIcon(QIcon::fromTheme("document-save-as"));
    saveAsAction->setEnabled(false); // will enable it later when file is valid
    connect(saveAsAction, SIGNAL(triggered()),
            this, SLOT(saveImage()));

    QToolBar *toolBar = new QToolBar;
    toolBar->addAction(closeAction);
    toolBar->addAction(saveAsAction);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, toolBar);

    // statusbar:
    QFileInfo info(file);
    QString infoString = info.fileName()
            + " (" + readableFileSize(info.size()) + ')';
    QLabel *fileInfoLabel = new QLabel;
    fileInfoLabel->setText(infoString);
    statusBar()->addPermanentWidget(fileInfoLabel);

    // try reading svg from file:
    QByteArray imageData;
    if (info.exists() && file.endsWith(".svg", Qt::CaseInsensitive)) {
        QFile f(file);
        f.open(QIODevice::ReadOnly);
        if (f.isOpen())
            imageData = f.readAll();
        f.close();
    }

    // file reading failed somehow?
    if (imageData.isEmpty()) {
        // create error/dummy svg:
        QTextStream stream(&imageData, QIODevice::WriteOnly | QIODevice::Truncate);
        stream << "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n"
               << "  <rect width=\"250\" height=\"250\" fill=\"rgb(234,234,234)\" stroke-width=\"1\" stroke=\"rgb(0,0,0)\"/>\n"
               << "  <text x=\"125\" y=\"125\" style=\"font-size:20px\" text-anchor=\"middle\">\n"
               << "    Error Loading Image\n"
               << "  </text>\n"
               << "</svg>";
    } else {
        // enable saving:
        saveAsAction->setEnabled(true);
        m_file = file;
        // adds '*' to show that it is unsaved and makes closeEvent() complain:
        setWindowModified(true);
    }

    // central widget:
    QSvgWidget *svgWidget = new QSvgWidget;
    svgWidget->load(imageData);
    setCentralWidget(svgWidget);

    // widget attributes:
    setAttribute(Qt::WA_DeleteOnClose); // auto delete this class
}

void SvgViewer::closeEvent(QCloseEvent *event)
{
    if (isWindowModified()) {
        // prepare questions and information:
        QString title = tr("Close Plot - %1").arg(qApp->applicationName());
        QString text = tr("The plot \"%1\" has not been saved.").arg(m_title);
        QString infoText = tr("Do you want to save as SVG image or discard it?");

        // construct dialog:
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(title);
        msgBox.setText(text);
        msgBox.setInformativeText(infoText);
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                                  QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        // show dialog and evaluate answer:
        auto ret = msgBox.exec();
        if (ret == QMessageBox::Cancel) {
            event->ignore();
            return;
        } else if (ret == QMessageBox::Save) {
            bool saved = saveImage();
            if (!saved) {
                event->ignore();
                return;
            }
        }
    }

    event->accept();
}

QString SvgViewer::readableFileSize(qint64 fileSize) const
{
    double fileSizeInBytes = fileSize;
    char byteUnitPrefixes[] = {'k', 'M', 'G', 'T'};
    int i = -1;
    do {
        fileSizeInBytes /= 1024.;
        i++;
    } while ((fileSizeInBytes > 1024.) && (i < 3));

    return QString("%L1%2B").arg(fileSizeInBytes, 0, 'f', 2)
            .arg(byteUnitPrefixes[i]);
}

bool SvgViewer::saveImage()
{
    if (m_file.isEmpty())
        return false;

    QString title = tr("Save File - %1").arg(qApp->applicationName());
    QDir dir("../data");
    QString proposedFile = tr("%1/%2").arg(dir.canonicalPath()).arg(m_file);
    QString filter = tr("Scalable Vector Graphics(*.svg)") + ";;";
    filter += tr("All Files(*.*)");
    QString selectedFilter;

#ifdef QT_HAS_IOCOMPRESSOR
    if (QtIOCompressor::isGzipSupported())
        filter = tr("Compressed Scalable Vector Graphics(*.svgz)") + ";;" + filter;
#endif

    QString newFile = QFileDialog::getSaveFileName(this,
                                                   title,
                                                   proposedFile,
                                                   filter,
                                                   &selectedFilter);
    if (newFile.isEmpty()) // save aborted?
        return false;

    if (QFile::exists(newFile)) // files does exist?
        QFile::remove(newFile); // ... remove it, otherwise ::copy will fail
    bool success = false;
#ifdef QT_HAS_IOCOMPRESSOR
    if (QtIOCompressor::isGzipSupported() &&
            (selectedFilter == tr("Compressed Scalable Vector Graphics(*.svgz)"))) {

        QFile in(m_file);
        in.open(QIODevice::ReadOnly);

        QFile out(newFile);
        QtIOCompressor compressor(&out);
        compressor.setStreamFormat(QtIOCompressor::GzipFormat);
        compressor.open(QIODevice::WriteOnly);
        qint64 s = compressor.write(in.readAll());

        success = (s == in.size());
    } else
#endif
    success = QFile::copy(m_file, newFile);
    if (!success) // copy or compression failed?
        statusBar()->showMessage(tr("Saving Failed!"), 5000);
    else
        setWindowModified(false); // removes trailing star in windowtitle

    return success;
}

#include "svgviewer.moc"
