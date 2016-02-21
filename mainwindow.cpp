#include "mainwindow.h"
#include "botviewdelegate.h"
#include "spinbox.h"

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
#include <QPushButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include <QTimer>
#include <QLabel>
#include <QThread>
#include <QTime>

#define SERVERPATH_KEY    "serverpath"
#define ROUNDS_KEY        "rounds"
#define AUTOSTART_KEY     "autostart"
#define AUTOQUIT_KEY      "autoquit"
#define TICKINTERVAL_KEY  "tickinterval"
#define FULLSCREEN_KEY    "fullscreen"
#define HEADLESS_KEY      "headless"
#define ROUNDSPLAYED_KEY  "headless"

static MainWindow *instance = 0;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();

    if (instance) {
        switch (type) {
        case QtWarningMsg:
        case QtCriticalMsg:
            instance->errorOutput(msg);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            abort();
        case QtDebugMsg:
        default:
            instance->normalOutput(msg);
            break;
        }
    }
    switch (type) {
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    case QtDebugMsg:
    default:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QSplitter(parent),
      m_botsView(new QTableView(this)),
      m_botModel(new BotModel(this)),
      m_logFile("BORG.log")
{
    m_logFile.open(QIODevice::WriteOnly | QIODevice::Append);

    instance = this;
    qInstallMessageHandler(messageHandler);

    QSettings settings;
    m_roundsPlayed = settings.value(ROUNDSPLAYED_KEY, 0).toInt();

    // Left part of window
    QWidget *leftWidget = new QWidget;
    QLayout *leftLayout = new QVBoxLayout;
    leftWidget->setLayout(leftLayout);
    addWidget(leftWidget);

    // Round names
    QFile nameFile("names.txt");
    if (nameFile.open(QIODevice::ReadOnly)) {
        m_names = nameFile.readAll().trimmed().split('\n');
        qsrand(QTime::currentTime().msec());
        QFont f( "Arial", 20, QFont::Bold);
        m_name.setFont(f);
    } else {
        qDebug() << "Unable to open names file" << nameFile.errorString();
    }
    m_name.setText("...");
    updateName();
    leftLayout->addWidget(&m_name);

    ///////////
    /// Bot list view
    ///
    m_botsView->setItemDelegate(new BotViewDelegate);
    m_botsView->setModel(m_botModel);
    m_botsView->setShowGrid(false);
    m_botsView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_botsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_botsView->resizeColumnsToContents();
    m_botsView->horizontalHeader()->setStretchLastSection(true);
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
    serverBox->setLayout(new QVBoxLayout);
    leftLayout->addWidget(serverBox);

    ///////////
    /// Server launch
    ///
    QWidget *serverLaunch = new QWidget;
    serverLaunch->setLayout(new QHBoxLayout);
    serverBox->layout()->addWidget(serverLaunch);
    // Launch button
    m_launchButton = new QPushButton(tr("&Start server"));
    serverLaunch->layout()->addWidget(m_launchButton);
    // Server path editor
    m_serverPath = new PathEditor,
    serverLaunch->layout()->addWidget(m_serverPath);
    m_serverPath->setPath(settings.value(SERVERPATH_KEY, "").toString());
    // Kill button
    QPushButton *killButton = new QPushButton(tr("&Kill server"));
    serverLaunch->layout()->addWidget(killButton);

    ///////////
    /// Server settings
    ///
    QWidget *serverSettings = new QWidget;
    serverSettings->setLayout(new QHBoxLayout);
    serverBox->layout()->addWidget(serverSettings);
    // Round count editor
    m_rounds = new QSpinBox;
    m_rounds->setMinimum(1);
    m_rounds->setMaximum(10);
    m_rounds->setSuffix(tr(" rounds"));
    serverSettings->layout()->addWidget(m_rounds);
    m_rounds->setValue(settings.value(ROUNDS_KEY, 4).toInt());
    // Tick interval editor
    m_tickInterval = new QSpinBox;
    m_tickInterval->setMinimum(10);
    m_tickInterval->setMaximum(1000);
    m_tickInterval->setSuffix(" ms ticks");
    serverSettings->layout()->addWidget(m_tickInterval);
    m_tickInterval->setValue(settings.value(TICKINTERVAL_KEY, 50).toInt());
    // Autostart checkbox
    m_autoLaunch = new QCheckBox(tr("Start automatically"));
    serverSettings->layout()->addWidget(m_autoLaunch);
    m_autoLaunch->setChecked(settings.value(AUTOSTART_KEY, false).toBool());
    // Autoquit checkbox
    m_autoQuit = new QCheckBox(tr("Quit on game over"));
    serverSettings->layout()->addWidget(m_autoQuit);
    m_autoQuit->setChecked(settings.value(AUTOQUIT_KEY, false).toBool());
    // Fullscreen checkbox
    m_fullscreen = new QCheckBox(tr("Fullscreen"));
    serverSettings->layout()->addWidget(m_fullscreen);
    m_fullscreen->setChecked(settings.value(FULLSCREEN_KEY, false).toBool());
    // Headless checkbox
    m_headless = new QCheckBox(tr("Headless mode"));
    serverSettings->layout()->addWidget(m_headless);
    m_headless->setChecked(settings.value(HEADLESS_KEY, false).toBool());

    // Some spacing to align things to the left
    serverSettings->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    // Some spacing to quit and kill buttons
    leftLayout->addItem(new QSpacerItem(0, 50));

    ///////////
    /// Quit button
    ///
    QPushButton *quitButton = new QPushButton(tr("&Quit"));
    connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
    leftLayout->addWidget(quitButton);

    ///////////
    /// Right part of window
    ///
    QWidget *rightWidget = new QWidget;
    rightWidget->setLayout(new QVBoxLayout);
    addWidget(rightWidget);
    // Top players list
    m_topPlayers.setText("<h3>Top players</h3><ol><li>...</li></ol>");
    rightWidget->layout()->addWidget(&m_topPlayers);
    updateTopPlayers();
    // Log/output view
    m_serverOutput.setReadOnly(true);
    rightWidget->layout()->addWidget(&m_serverOutput);
    // Reset button
    QPushButton *resetButton = new QPushButton(tr("Reset"));
    rightWidget->layout()->addWidget(resetButton);

    // Connections
    connect(killButton, SIGNAL(clicked()), SLOT(kill()));

    connect(m_serverPath, SIGNAL(pathChanged(QString)), SLOT(saveSettings()));
    connect(m_rounds, SIGNAL(valueChanged(int)), SLOT(saveSettings()));
    connect(m_autoLaunch, SIGNAL(stateChanged(int)), SLOT(saveSettings()));
    connect(m_autoQuit, SIGNAL(stateChanged(int)), SLOT(saveSettings()));
    connect(m_tickInterval, SIGNAL(valueChanged(int)), SLOT(saveSettings()));
    connect(m_fullscreen, SIGNAL(stateChanged(int)), SLOT(saveSettings()));
    connect(m_headless, SIGNAL(stateChanged(int)), SLOT(saveSettings()));

    connect(m_launchButton, SIGNAL(clicked()), SLOT(launchServer()));
    connect(resetButton, SIGNAL(clicked()), SLOT(resetBots()));
    connect(&m_serverProcess, SIGNAL(readyReadStandardError()), SLOT(readServerErr()));
    connect(&m_serverProcess, SIGNAL(readyReadStandardOutput()), SLOT(readServerOut()));
    connect(&m_serverProcess, SIGNAL(finished(int)), SLOT(serverFinished(int)));

    resize(1920, 1200);
}

MainWindow::~MainWindow()
{
    kill();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue(SERVERPATH_KEY, m_serverPath->path());
    settings.setValue(ROUNDS_KEY, m_rounds->value());
    settings.setValue(AUTOSTART_KEY, m_autoLaunch->isChecked());
    settings.setValue(AUTOQUIT_KEY, m_autoQuit->isChecked());
    settings.setValue(TICKINTERVAL_KEY, m_tickInterval->value());
    settings.setValue(FULLSCREEN_KEY, m_fullscreen->isChecked());
    settings.setValue(HEADLESS_KEY, m_headless->isChecked());
    settings.setValue(ROUNDSPLAYED_KEY, m_roundsPlayed);
}

void MainWindow::launchServer()
{
    if (m_botModel->enabledPlayers() > 4 || m_botModel->enabledPlayers() < 1) {
        QMessageBox::warning(this, tr("Invalid number of players"), tr("Either too few or too many players enabled"));
        return;
    }

    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
    m_logFile.setFileName(QDateTime::currentDateTime().toString(Qt::ISODate) + ".log");
    m_logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    m_logFile.write("Starting " + m_name.text().toUtf8() + "\n");

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
    arguments << "--tick-interval" << QString::number(m_tickInterval->value());
    arguments << "--rounds" << QString::number(m_rounds->value());

    if (m_autoLaunch->isChecked()) {
        arguments << "--start-at" << QString::number(m_botModel->enabledPlayers());
    }
    if (m_autoQuit->isChecked()) {
        arguments << "--quit-on-finish";
    }
    if (m_fullscreen->isChecked()) {
        arguments << "--fullscreen";
    }
    if (m_headless->isChecked()) {
        arguments << "--headless";
    }

    m_serverProcess.setWorkingDirectory(serverExecutable.path());
    m_serverProcess.start(serverExecutable.filePath(), arguments);

    QTimer::singleShot(1000, m_botModel, SLOT(launchBots()));
}

void MainWindow::readServerErr()
{
    QByteArray output = m_serverProcess.readAllStandardError();
    errorOutput(output);
}

void MainWindow::readServerOut()
{
    QByteArray output = m_serverProcess.readAllStandardOutput();
    normalOutput(output);
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
    m_serverProcess.terminate();
    QThread::usleep(200);
    m_serverProcess.kill();
    m_botModel->killBots();
}

void MainWindow::serverFinished(int status)
{
    m_botModel->killBots();

    m_logFile.close();
    m_logFile.setFileName("BORG.log");
    m_logFile.open(QIODevice::WriteOnly | QIODevice::Append);

    qWarning() << m_name.text() << "finished";

    if (status != 0) {
        qWarning() << "Server finished with unclean status" << status;
        m_serverOutput.append(QStringLiteral("Server finished with unclean status %1!\n").arg(status));
    }
    QFile resultsLog(m_serverProcess.workingDirectory() + "/scores.txt");
    if (!resultsLog.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Unable to open server log"), tr("Unable to open server log with results"));
        qWarning() << "unable to open results log!";
        return;
    }

    int playersRead = 0;
    while (!resultsLog.atEnd()) {
        QList<QByteArray> player = resultsLog.readLine().split(' ');
        if (player.length() != 4) {
            QMessageBox::warning(this, tr("Invalid scores.txt"), tr("The scores.txt file is corrupt (invalid column), please adjust scores manually."));
            return;
        }

        bool roundWinsValid = false;
        bool scoreValid = false;
        m_botModel->roundOver(QString::fromUtf8(player[0]), (playersRead == 0), player[1].toInt(&roundWinsValid), player[2].toInt(&scoreValid));

        if (!roundWinsValid || !scoreValid) {
            QMessageBox::warning(this, tr("Invalid scores.txt"), tr("The scores.txt file is corrupt (invalid numbers), please adjust scores manually."));
            return;
        }
        playersRead++;
    }

    if (playersRead != m_botModel->enabledPlayers()) {
        QMessageBox::warning(this, tr("Missing players"), tr("Missing players from the scores.txt, please adjust manually"));
    }

    m_roundsPlayed++;
    updateTopPlayers();
    updateName();
    saveSettings();
}

void MainWindow::updateName()
{
    if (m_names.isEmpty()) {
        return;
    }
    m_name.setText("Game: " + m_names[qrand() % m_names.size()] + " (" + QString::number(m_roundsPlayed) + ")");
}

void MainWindow::resetBots()
{
    if (QMessageBox::question(this, tr("Really reset?"), tr("Are you sure you want to reset everything?")) == QMessageBox::No) {
        return;
    }
    m_roundsPlayed = 0;
    saveSettings();
    m_botModel->resetBots();
    updateName();
    updateTopPlayers();
}

void MainWindow::updateTopPlayers()
{
    QString labelContent("<h3>Top players:</h3><ol>");

    foreach(const QString name, m_botModel->topPlayers()) {
        labelContent += QString("<li>%1</li>").arg(name);
    }

    labelContent += "</ol><br>";
    m_topPlayers.setText(labelContent);
}

void MainWindow::errorOutput(QString message)
{
    QColor oldColor = m_serverOutput.textColor();
    m_serverOutput.setTextColor(Qt::red);
    m_serverOutput.append(message);
    m_serverOutput.setTextColor(oldColor);

    if (m_logFile.isOpen()) {
        m_logFile.write(message.toUtf8() + "\n");
    }
}

void MainWindow::normalOutput(QString message)
{
    m_serverOutput.append(message);

    if (m_logFile.isOpen()) {
        m_logFile.write(message.toUtf8() + "\n");
    }
}
