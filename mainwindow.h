#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSplitter>
#include <QTableView>
#include <QProcess>
#include <QTextEdit>
#include "botmodel.h"
#include "patheditor.h"

class QSpinBox;
class QPushButton;

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

private:
    QTableView *m_botsView;
    BotModel *m_botModel;
    PathEditor *m_serverPath;
    QSpinBox *m_rounds;
    PathEditor *m_mapPath;
    QPushButton *m_launchButton;
    QProcess m_serverProcess;
    QTextEdit m_serverOutput;
};

#endif // MAINWINDOW_H
