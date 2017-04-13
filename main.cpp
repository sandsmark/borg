#include "mainwindow.h"
#include <QApplication>
#include <QQmlEngine>

#include "botmodel.h"
#include "tournamentcontroller.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<Bot>("Bot");
    qRegisterMetaTypeStreamOperators<Bot>("Bot");
    qRegisterMetaTypeStreamOperators<QList<Bot> >("QList<Bot>");

    QApplication a(argc, argv);
    a.setApplicationName("BORG");
    a.setOrganizationName("Martin Sandsmark");
    a.setOrganizationDomain("iskrembilen.com");

    qmlRegisterType<Round>();
    qmlRegisterType<Match>();
    qmlRegisterType<Competitor>();
    qmlRegisterSingletonType<TournamentController>("com.iskrembilen", 1, 0, "TournamentController", [](QQmlEngine *engine, QJSEngine*) -> QObject* {
        engine->setObjectOwnership(TournamentController::instance(), QQmlEngine::CppOwnership);
        return TournamentController::instance();
    });

    MainWindow w;


    w.show();

    return a.exec();
}
