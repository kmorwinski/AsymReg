#include "svgviewer.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>

#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>

#include <QtSvg/QSvgWidget>

SvgViewer::SvgViewer(const QString &file, const QString &title)
    : m_saved(false)
{
    // window title:
    static int _num = 0; // running number
    setWindowTitle(tr("Plot %1: ").arg(++_num) + title);

    // toolbar:
    QAction *closeAction = new QAction(this);
    closeAction->setText(tr("&Close"));
    closeAction->setIcon(QIcon::fromTheme("window-close"));
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
    addToolBar(Qt::TopToolBarArea, toolBar);

    // statusbar:
    QFileInfo info(file);
    QLabel *nameLabel = new QLabel;
    nameLabel->setText(tr("Filename: ") + info.fileName());
    statusBar()->addPermanentWidget(nameLabel);

    QLabel *sizeLabel = new QLabel;
    sizeLabel->setText(tr("Size: %1b").arg(info.size()));
    statusBar()->addPermanentWidget(sizeLabel);

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
        setWindowTitle(windowTitle() + '*'); // add '*' to show that it is unsaved
        m_file = file;
    }

    // central widget:
    QSvgWidget *svgWidget = new QSvgWidget;
    svgWidget->load(imageData);
    setCentralWidget(svgWidget);

    // widget attributes:
    setAttribute(Qt::WA_DeleteOnClose); // auto delete this class
}

void SvgViewer::saveImage()
{
    if (m_file.isEmpty())
        return;

    // remove trailing star:
    QString t = windowTitle();
    if (t.endsWith('*'))
        setWindowTitle(t.left(t.size()-1));

    m_saved = true;
}

#include "svgviewer.moc"
