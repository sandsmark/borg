#include "mainwindow.h"
#include <QApplication>
#include "botmodel.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<Bot>("Bot");
    qRegisterMetaTypeStreamOperators<Bot>("Bot");
    qRegisterMetaTypeStreamOperators<QList<Bot> >("QList<Bot>");

    QApplication a(argc, argv);
    a.setApplicationName("BORG");
    a.setOrganizationName("Martin Sandsmark");
    a.setOrganizationDomain("iskrembilen.com");
    MainWindow w;
    w.show();

    return a.exec();
}
