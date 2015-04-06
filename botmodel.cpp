#include "botmodel.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QSettings>

#define BOTS_KEY "bots"

BotModel::BotModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    QSettings settings;
    m_bots = settings.value(BOTS_KEY, QVariant::fromValue(QList<Bot>())).value<QList<Bot> >();
    for (int i=0; i<m_bots.length(); i++) {
        initializeBotProcess(&m_bots[i]);
    }
}

BotModel::~BotModel()
{
    killBots();
}

QVariant BotModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    Column column = (Column)index.column();

    if (row > m_bots.length()) {
        qWarning() << "asked for illegal row" << row;
        return QVariant();
    }

    if (role == Qt::CheckStateRole && column == Enabled) {
        return m_bots[row].enabled ? Qt::Checked : Qt::Unchecked;
    } else if (role != Qt::DisplayRole) {
        return QVariant();
    }

    const Bot &bot = m_bots.at(row);

    switch (column) {
    case Enabled:
        return bot.enabled ? tr("Enabled") : tr("Disabled");
    case Name:
        return bot.name;
    case Runtime:
        return bot.runtime;
    case Path:
        return bot.path;
    case Arguments:
        return bot.arguments;
    case Wins:
        return bot.wins;
    case RoundsPlayed:
        return bot.roundsPlayed;
    case Running:
        switch(bot.process->state()) {
        case QProcess::Running:
            return tr("Running");
        case QProcess::Starting:
            return tr("Starting");
        case QProcess::NotRunning:
            return tr("Not running");
        default:
            return tr("Unknown");
        }
        break;
    default:
        qWarning() << Q_FUNC_INFO << "asked for unknown colum" << column;
        return QVariant("fuck off");
    }
}

QVariant BotModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }
    switch (section) {
    case Enabled:
        return tr("Enabled");
    case Name:
        return tr("Name");
    case Runtime:
        return tr("Runtime");
    case Path:
        return tr("Path");
    case Arguments:
        return tr("Arguments");
    case Wins:
        return tr("Wins");
    case RoundsPlayed:
        return tr("Rounds Played");
    case Running:
        return tr("Running");
    default:
        qDebug() << "asked for unknown column" << section;
        return QVariant("Foo");
    }
}

bool BotModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);

    int row = index.row();
    if (row > m_bots.length()) {
        qWarning() << "asked for illegal row" << row;
        return false;
    }
    Bot *bot = &m_bots[row];

    switch(index.column()) {
    case Enabled:
        bot->enabled = value.toBool();
        break;
    case Name:
        bot->name = value.toString();
        break;
    case Runtime:
        bot->runtime = value.toString();
        break;
    case Path:
        bot->path = value.toString();
        break;
    case Arguments:
        bot->arguments = value.toString();
    case Wins:
        bot->wins = value.toInt();
        break;
    case RoundsPlayed:
        bot->roundsPlayed = value.toInt();
    case Running:
        return false;
    default:
        qWarning() << "asked to set value for illegal column" << index.column();
        return false;
    }
    save();
    return true;
}

Qt::ItemFlags BotModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == Enabled) {
        flags |= Qt::ItemIsUserCheckable;
    }

    if (index.column() != Running) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

void BotModel::removeRow(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    m_bots.removeAt(row);
    endRemoveRows();
    save();
}

void BotModel::launchBots()
{
    beginResetModel(); // For running info
    for (int i=0; i<m_bots.length(); i++) {
        if (!m_bots[i].enabled) continue;
        QFileInfo file(m_bots[i].path);
        m_bots[i].process->setWorkingDirectory(file.path());
        if (m_bots[i].runtime.isEmpty()) {
            m_bots[i].process->start(m_bots[i].path);
        } else {
            if (!runtimes().contains(m_bots[i].runtime)) {
                qWarning() << "Unable to find runtime" << m_bots[i].runtime;
                continue;
            }
            QStringList arguments;
            if (m_bots[i].runtime == "java") {
                arguments << "-jar";
            }
            arguments << m_bots[i].path;
            arguments << m_bots[i].arguments.split(' ');
            qDebug() << "launching" << runtimes()[m_bots[i].runtime] << arguments;
            m_bots[i].process->start(runtimes()[m_bots[i].runtime], arguments);
        }
    }
    endResetModel();
}

void BotModel::killBots()
{
    beginResetModel();
    for (int i=0; i<m_bots.length(); i++) {
        m_bots[i].process->kill();
    }
    endResetModel();
}

void BotModel::handleProcessError(QProcess::ProcessError error)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (process) {
        qWarning() << "Error from bot: " << process->program();
    }
    switch(error){
    case QProcess::FailedToStart:
        qWarning() << "Bot failed to start";
        break;
    case QProcess::Crashed:
        qWarning() << "Bot crashed";
        break;
    case QProcess::Timedout:
        qWarning() << "Bot launch timed out";
        break;
    case QProcess::WriteError:
        qWarning() << "Write error when communicating with bot";
        break;
    case QProcess::ReadError:
        qWarning() << "Read error when communicating with bot";
    case QProcess::UnknownError:
    default:
        qWarning() << "Unknown bot process error";
    }
}

void BotModel::initializeBotProcess(Bot *bot)
{
    bot->process = QSharedPointer<QProcess>(new QProcess);
    QFileInfo info(bot->path);
    bot->logfile = QSharedPointer<QFile>(new QFile(info.path() + "/logfile.txt"));
    bot->logfile->open(QIODevice::WriteOnly | QIODevice::Append);
    connect(bot->process.data(), SIGNAL(readyReadStandardError()), SLOT(storeOutput()));
    connect(bot->process.data(), SIGNAL(readyReadStandardOutput()), SLOT(storeOutput()));
    connect(bot->process.data(), SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(botStateChanged()));
    connect(bot->process.data(), SIGNAL(error(QProcess::ProcessError)), SLOT(handleProcessError(QProcess::ProcessError)));
}

void BotModel::addBot(QString path)
{
    QFileInfo file(path);

    if (!file.exists()) {
        QMessageBox::warning(0, tr("Unable to add bot"), tr("The path to the bot does not exist."));
        return;
    }

    Bot bot;
    bot.enabled = false;
    bot.path = path;
    bot.name = file.dir().dirName();
    bot.wins = 0;
    bot.roundsPlayed = 0;
    if (file.suffix() == "py") {
        bot.runtime = "python";
    } else if (file.suffix() == "rb") {
        bot.runtime = "ruby";
    } else if (file.suffix() == "js") {
        bot.runtime = "nodejs";
    } else if (file.suffix() == "pl") {
        bot.runtime = "perl";
    } else if (file.suffix() == "exe") {
        bot.runtime = "mono";
    } else if (file.suffix() == "jar") {
        bot.runtime = "java";
    } else if (!file.isExecutable()) {
        bot.runtime = "unknown";
    }
    beginInsertRows(QModelIndex(), m_bots.length(), m_bots.length() + 1);
    m_bots.append(bot);
    initializeBotProcess(&m_bots.last());
    endInsertRows();

    save();
}

void BotModel::save()
{
    QSettings settings;
    settings.setValue(BOTS_KEY, QVariant::fromValue(m_bots));
}

QHash<QString, QString> BotModel::runtimes()
{
    QHash<QString, QString> ret;
    ret["python"] = "/usr/bin/python2";
    ret["python3"] = "/usr/bin/python";
    ret["ruby"] = "/usr/bin/ruby";
    ret["nodejs"] = "/usr/bin/node";
    ret["perl"] = "/usr/bin/perl";
    ret["mono"] = "/usr/bin/mono";
    ret["java"] = "/usr/bin/java";

    return ret;
}

void BotModel::roundOver(QString name)
{
    for(int i=0; i<m_bots.length(); i++) {
        if (m_bots[i].enabled) {
            m_bots[i].roundsPlayed++;
        }
    }

    for(int i=0; i<m_bots.length(); i++) {
        if (!m_bots[i].enabled) continue;
        if (m_bots[i].name != name) continue;

        m_bots[i].wins++;
        save();
        return;
    }

    QMessageBox::warning(0, tr("Unable to find winner!"), tr("Unable to find '%1' in the list of bots, please adjust score manually").arg(name));
}

int BotModel::enabledPlayers()
{
    int ret = 0;
    foreach(const Bot &bot, m_bots) {
        if (bot.enabled) {
            ret++;
        }
    }
    return ret;
}



QDataStream &operator<<(QDataStream &out, const Bot &bot)
{
    out << bot.enabled << bot.name << bot.path << bot.arguments << bot.runtime << bot.wins << bot.roundsPlayed;
    return out;
}


QDataStream &operator>>(QDataStream &in, Bot &bot)
{
    in >> bot.enabled >> bot.name >> bot.path >> bot.arguments >> bot.runtime >> bot.wins >> bot.roundsPlayed;
    return in;
}


void BotModel::storeOutput()
{
    for (int i=0; i<m_bots.length(); i++) {
        if (m_bots[i].process.data() != sender()) continue;

        QByteArray err = m_bots[i].process->readAllStandardError();
        if (!err.isEmpty()) {
        //    qWarning() << m_bots[i].process->program() << err;
        }
        m_bots[i].logfile->write(err);
        QByteArray out = m_bots[i].process->readAllStandardOutput();
        if (!out.isEmpty()) {
        //    qDebug() << m_bots[i].process->program() << out;
        }
        m_bots[i].logfile->write(out);
        return;
    }
    qWarning() << "Unable to find bot with process" << sender();
}

void BotModel::botStateChanged()
{
    //TODO: actually check which bots changed and whatnot plz
    beginResetModel();
    endResetModel();
}
