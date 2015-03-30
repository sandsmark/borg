#include "mainwindow.h"
#include "botviewdelegate.h"
#include <QHeaderView>
#include <QItemDelegate>
#include <QItemEditorFactory>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QGroupBox>
#include <QSettings>
#include <QSpinBox>
#include <QPushButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QFileDialog>
#include <QTimer>

#define SERVERPATH_KEY "serverpath"
#define PLAYERS_KEY    "players"
#define ROUNDS_KEY     "rounds"
#define MAPPATH_KEY    "mappath"

MainWindow::MainWindow(QWidget *parent)
    : QSplitter(parent),
      m_botsView(new QTableView(this)),
      m_botModel(new BotModel(this)),
      m_serverPath(new PathEditor),
      m_players(new QSpinBox),
      m_rounds(new QSpinBox),
      m_mapPath(new PathEditor),
      m_launchButton(new QPushButton(tr("&Launch server")))
{
    QWidget *leftWidget = new QWidget;
    QLayout *leftLayout = new QVBoxLayout;
    leftWidget->setLayout(leftLayout);
    addWidget(leftWidget);

    m_serverOutput.setReadOnly(true);
    addWidget(&m_serverOutput);

    ///////////
    /// Bot list view
    ///
    m_botsView->setItemDelegate(new BotViewDelegate);
    m_botsView->horizontalHeader()->show();
    m_botsView->setModel(m_botModel);
    m_botsView->resizeColumnsToContents();
    m_botsView->setShowGrid(false);
    m_botsView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_botsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    leftLayout->addWidget(m_botsView);

    ///////////
    /// Add/remove buttons
    ///
    QWidget *addRemoveGroup = new QWidget;
    addRemoveGroup->setLayout(new QHBoxLayout);
    QPushButton *addButton = new QPushButton(tr("Add"));
    connect(addButton, SIGNAL(clicked()), SLOT(addBot()));
    addRemoveGroup->layout()->addWidget(addButton);
    QPushButton *removeButton = new QPushButton(tr("Remove"));
    connect(removeButton, SIGNAL(clicked()), SLOT(removeBot()));
    addRemoveGroup->layout()->addWidget(removeButton);
    leftLayout->addWidget(addRemoveGroup);

    ///////////
    /// Server control
    ///
    QGroupBox *serverBox = new QGroupBox(tr("Server"));
    serverBox->setLayout(new QHBoxLayout);
    serverBox->layout()->addWidget(m_serverPath);
    m_players->setMinimum(1);
    m_players->setMaximum(8);
    serverBox->layout()->addWidget(m_players);
    m_rounds->setMinimum(1);
    m_rounds->setMaximum(10);
    serverBox->layout()->addWidget(m_rounds);
    serverBox->layout()->addWidget(m_mapPath);
    serverBox->layout()->addWidget(m_launchButton);
    leftLayout->addWidget(serverBox);


    leftLayout->addItem(new QSpacerItem(0, 50));

    ///////////
    /// Kill button
    ///
    QPushButton *killButton = new QPushButton(tr("&Kill"));
    connect(killButton, SIGNAL(clicked()), SLOT(kill()));
    leftLayout->addWidget(killButton);

    ///////////
    /// Quit button
    ///
    QPushButton *quitButton = new QPushButton(tr("&Quit"));
    connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
    leftLayout->addWidget(quitButton);

    QSettings settings;
    m_serverPath->setPath(settings.value(SERVERPATH_KEY, "").toString());
    m_players->setValue(settings.value(PLAYERS_KEY, 4).toInt());
    m_rounds->setValue(settings.value(ROUNDS_KEY, 4).toInt());
    m_mapPath->setPath(settings.value(MAPPATH_KEY, "map1.map").toString());

    connect(m_serverPath, SIGNAL(pathChanged(QString)), SLOT(saveSettings()));
    connect(m_mapPath, SIGNAL(pathChanged(QString)), SLOT(saveSettings()));
    connect(m_players, SIGNAL(valueChanged(int)), SLOT(saveSettings()));
    connect(m_rounds, SIGNAL(valueChanged(int)), SLOT(saveSettings()));
    connect(m_launchButton, SIGNAL(clicked()), SLOT(launchServer()));
    connect(&m_serverProcess, SIGNAL(readyReadStandardError()), SLOT(readServerErr()));
    connect(&m_serverProcess, SIGNAL(readyReadStandardOutput()), SLOT(readServerOut()));
    connect(&m_serverProcess, SIGNAL(finished(int)), SLOT(serverFinished(int)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue(SERVERPATH_KEY, m_serverPath->path());
    settings.setValue(PLAYERS_KEY, m_players->value());
    settings.setValue(ROUNDS_KEY, m_rounds->value());
    settings.setValue(MAPPATH_KEY, m_mapPath->path());
}

void MainWindow::launchServer()
{
    if (m_serverProcess.state() == QProcess::Running) {
        QMessageBox::warning(this, tr("Server already running"), tr("The server executable is still running"));
        return;
    }

    QFileInfo serverExecutable(m_serverPath->path());
    if (!serverExecutable.exists()) {
        QMessageBox::warning(this, tr("Server not found"), tr("Can't find the server at the path provided!"));
        return;
    }

    if (!serverExecutable.isExecutable()) {
        QMessageBox::warning(this, tr("Server not runnable"), tr("The server is not an executable file!"));
        return;
    }

    QStringList arguments;
    arguments << "server"
              << QString::number(m_players->value())
              << QString::number(m_rounds->value());

    QFileInfo mapFile(m_mapPath->path());
    if (mapFile.exists()) {
        arguments.append(mapFile.filePath());
    }

    m_serverProcess.setWorkingDirectory(serverExecutable.path());
    m_serverProcess.start(serverExecutable.filePath(), arguments);

    QTimer::singleShot(1000, m_botModel, SLOT(launchBots()));
}

void MainWindow::readServerErr()
{
    QByteArray output = m_serverProcess.readAllStandardError();
    QColor oldColor = m_serverOutput.textColor();
    m_serverOutput.setTextColor(Qt::red);
    m_serverOutput.append(output);
    m_serverOutput.setTextColor(oldColor);
}

void MainWindow::readServerOut()
{
    QByteArray output = m_serverProcess.readAllStandardOutput();
    m_serverOutput.append(output);
}

void MainWindow::addBot()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select bot"));
    m_botModel->addBot(path);
}

void MainWindow::removeBot()
{
    const QModelIndex index = m_botsView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    const int row = index.row();
    QString name = m_botModel->data(m_botModel->index(row, BotModel::Name)).toString();
    if (QMessageBox::question(this, tr("Really remove?"), tr("Are you sure you want to remove %1?").arg(name)) == QMessageBox::No) {
        return;
    }
    m_botModel->removeRow(row);
}

void MainWindow::kill()
{
    m_serverProcess.kill();
    m_botModel->killBots();
}

void MainWindow::serverFinished(int status)
{
    m_botModel->killBots();

    if (status != 0) {
        qWarning() << "Server finished with unclean status" << status;
        m_serverOutput.append(QStringLiteral("Server finished with unclean status %1!\n").arg(status));
    }
    QFile resultsLog(m_serverProcess.workingDirectory() + "/scores.log");
    if (!resultsLog.open(QIODevice::ReadOnly)) {
        qWarning() << "unable to open results log!";
        m_serverOutput.append("Unable to open results log");
        return;
    }
    QByteArray winner = resultsLog.readLine().trimmed();
    m_botModel->roundOver(QString::fromUtf8(winner));
}
