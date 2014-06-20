#include <iostream>

#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtCore/QTranslator>

#include <QtGui/QApplication>

#include "mainwindow.h"

Q_DECLARE_METATYPE(QAction *) // needed in MainWindow::readSettings()

int main(int argc, char** argv)
{
    std::cout << std::fixed; // write floating-point values in fixed-point notation

    /* setup app name for QSettings: */
    QCoreApplication::setApplicationName("AsymReg");
    QCoreApplication::setOrganizationDomain("uni-due.de");
#if (defined Q_OS_WIN32) || (defined Q_OS_WIN64)
    QSettings::setDefaultFormat(QSettings::IniFormat); // do not use registry on windows
#endif

    /* register additional types and classes to Qt's meta-type system: */
    qRegisterMetaType<QAction *>("qaction_ptr"); // needed for delayed invocation of MainWindow::selectDataSource()

    /* load translations: */
    QTranslator stdTr; // for standard dialogs
    stdTr.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    // TODO: load our own translations too

    /* create QApplication instance: */
    QApplication app(argc, argv);
    app.installTranslator(&stdTr);

    /* create & show MainWindow instance: */
    MainWindow mw;
    mw.show();

    return app.exec(); // start event-queue
}
