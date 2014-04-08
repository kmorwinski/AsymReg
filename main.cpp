#include <QtCore/QSettings>
#include <QtGui/QApplication>

#include "mainwindow.h"

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName("AsymReg");
    QCoreApplication::setOrganizationDomain("uni-due.de");
#if (defined Q_OS_WIN32) || (defined Q_OS_WIN64)
    QSettings::setDefaultFormat(QSettings::IniFormat); // do not use registry on windows
#endif

    QApplication app(argc, argv);

    MainWindow mw;
    mw.show();
    return app.exec();
}
