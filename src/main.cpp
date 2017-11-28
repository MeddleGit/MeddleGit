#include "MainWindow.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationDomain("meddle.cf");
    QCoreApplication::setOrganizationName("MeddleGit");
    QCoreApplication::setApplicationName("MeddleGit");

    MainWindow w;
    w.show();

    return a.exec();
}
