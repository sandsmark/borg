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

    // Invert the palette, for reasons
    QPalette palette = QApplication::palette();

    for (int group = 0; group < QPalette::NColorGroups; group++) {
        QPalette::ColorGroup colorGroup = QPalette::ColorGroup(group);
        for (int role = 0; role < QPalette::NColorRoles; role++) {
            QPalette::ColorRole colorRole = QPalette::ColorRole(role);
            QColor color = palette.color(colorGroup, colorRole);
            QColor inverted(255 - color.red(), 255 - color.green(), 255 - color.blue(), color.alpha());
            palette.setColor(colorGroup, colorRole, inverted);
        }
    }
    QApplication::setPalette(palette);

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
