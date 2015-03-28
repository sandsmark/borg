#include "botmodel.h"
#include <QDebug>
BotModel::BotModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    for (int i=0; i<10; i++) {
        Bot bot;
        bot.enabled = i < 5;
        bot.name = QStringLiteral("bot") + QString::number(i);
        bot.runtime = QStringLiteral("python");
        bot.path = QStringLiteral("/usr/bin/foo");
        bot.wins = 0;
        m_bots.append(bot);
    }
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
    default:
        qWarning() << "asked for unknown colum" << column;
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
    default:
        qWarning() << "asked to set value for illegal column" << index.column();
        return false;
    }
    return true;
}

Qt::ItemFlags BotModel::flags(const QModelIndex &index) const
{
    if (index.column() == Enabled) {
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

