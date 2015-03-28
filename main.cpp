#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("BORG");
    a.setOrganizationName("Martin Sandsmark");
    a.setOrganizationDomain("iskrembilen.com");
    MainWindow w;
    w.show();

    return a.exec();
}
