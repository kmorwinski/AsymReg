#include <QtGui/QApplication>
#include "asymreg.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    asymreg foo;
    foo.show();
    return app.exec();
}
