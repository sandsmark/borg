#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSplitter>
#include <QTableView>
#include <QProcess>
#include <QTextEdit>
#include <QLabel>
#include "botmodel.h"
#include "patheditor.h"

class QSpinBox;
class QPushButton;
class QCheckBox;

class MainWindow : public QSplitter
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void errorOutput(QString message);
    void normalOutput(QString message);

private slots:
    void saveSettings();
    void launchServer();

    void readServerErr();
    void readServerOut();

    void addBot();
    void removeBot();

    void kill();
    void serverFinished(int status);

    void updateName();

private:
    QTableView *m_botsView;
    BotModel *m_botModel;
    PathEditor *m_serverPath;
    QSpinBox *m_rounds;
    QSpinBox *m_players;
    QSpinBox *m_tickInterval;
    QCheckBox *m_autoLaunch;
    QCheckBox *m_autoQuit;
    QCheckBox *m_fullscreen;
    QCheckBox *m_headless;
    QPushButton *m_launchButton;
    QProcess m_serverProcess;
    QTextEdit m_serverOutput;
    QList<QByteArray> m_names;
    QLabel m_name;
    QFile m_logFile;
};

#endif // MAINWINDOW_H
