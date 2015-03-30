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
}

BotModel::~BotModel()
{
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
    case Wins:
        return bot.wins;
    case Running:
        return bot.process->state() == QProcess::Running;
    default:
        qWarning() << Q_FUNC_INFO << "asked for unknown colum" << column;
        return QVariant("fuck off");
    }
}

QVariant BotModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return false;
    }
    if (orientation != Qt::Horizontal) {
        qWarning() << "sykt lol";
        return QVariant();
    }
    qDebug() << section;
    switch (section) {
    case Enabled:
        return tr("Enabled");
    case Name:
        return tr("Name");
    case Runtime:
        return tr("Runtime");
    case Path:
        return tr("Path");
    case Wins:
        return tr("Wins");
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
    case Wins:
        bot->wins = value.toInt();
        break;
    case Running:
        return false;
    default:
        qWarning() << "asked to set value for illegal column" << index.column();
        return false;
    }
    return true;
}

Qt::ItemFlags BotModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (index.column() == Enabled) {
        flags |= Qt::ItemIsUserCheckable;
    }

    if (index.column() != Running) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
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
    } else if (!file.isExecutable()) {
        bot.runtime = "unknown";
    }
    bot.process = new QProcess;
    beginInsertRows(QModelIndex(), m_bots.length(), m_bots.length() + 1);
    m_bots.append(bot);
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

    return ret;
}



QDataStream &operator<<(QDataStream &out, const Bot &bot)
{
    out << bot.enabled << bot.name << bot.path << bot.runtime << bot.wins;
    return out;
}


QDataStream &operator>>(QDataStream &in, Bot &bot)
{
    in >> bot.enabled >> bot.name >> bot.path >> bot.runtime >> bot.wins;
    bot.process = new QProcess;
    return in;
}
