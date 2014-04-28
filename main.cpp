#include <QtCore/QSettings>
#include <QtGui/QApplication>

#include "mainwindow.h"

Q_DECLARE_METATYPE(QAction *) // needed in MainWindow::readSettings()

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName("AsymReg");
    QCoreApplication::setOrganizationDomain("uni-due.de");
#if (defined Q_OS_WIN32) || (defined Q_OS_WIN64)
    QSettings::setDefaultFormat(QSettings::IniFormat); // do not use registry on windows
#endif

    qRegisterMetaType<QAction *>("qaction_ptr"); // register for QMetaMethod::invoke()

    QApplication app(argc, argv);

    MainWindow mw;
    mw.show();
    return app.exec();
}
