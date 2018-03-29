#include "botmodel.h"
#include "tournamentcontroller.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QSettings>
#include <algorithm>
#include <QThread>

#define BOTS_KEY "bots"

BotModel::BotModel() :
    m_tournamentMode(false)
{
    QSettings settings;
    m_bots = settings.value(BOTS_KEY, QVariant::fromValue(QList<Bot>())).value<QList<Bot>>();
}

BotModel::~BotModel()
{
}

QVariant BotModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    Column column = Column(index.column());

    if (row > m_bots.length()) {
        qWarning() << "asked for illegal row" << row;
        return QVariant();
    }

    if (role == Qt::CheckStateRole && column == Enabled) {
        return m_bots[row].enabled ? Qt::Checked : Qt::Unchecked;
    } else if (role == Qt::TextAlignmentRole && (column == Wins || column == RoundWins || column == TotalScore || column == RoundsPlayed)) {
        return Qt::AlignCenter;
    } else if (role != Qt::DisplayRole) {
        return QVariant();
    }

    const Bot &bot = m_bots.at(row);

    switch (column) {
    case Enabled:
        return QVariant();
    case Name:
        return bot.name;
    case Path:
        if (bot.path.length() > 20) {
            return QStringLiteral("...") + bot.path.right(20);
        } else {
            return bot.path;
        }
    case Wins:
        return bot.wins;
    case RoundWins:
        return bot.roundWins;
    case TotalScore:
        return bot.totalScore;
    case RoundsPlayed:
        return bot.roundsPlayed;
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
    case Path:
        return tr("Path");
    case Wins:
        return tr("Games won");
    case RoundWins:
        return tr("Rounds won");
    case TotalScore:
        return tr("Total points");
    case RoundsPlayed:
        return tr("Games Played");
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
    case Path:
        bot->path = value.toString();
        break;
    case Wins:
        bot->wins = value.toInt();
        break;
    case RoundsPlayed:
        bot->roundsPlayed = value.toInt();
        break;
    case TotalScore:
        bot->totalScore = value.toInt();
        break;
    case RoundWins:
        bot->roundWins = value.toInt();
        break;
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

    return flags;
}

void BotModel::removeRow(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    m_bots.removeAt(row);
    endRemoveRows();
    save();
}

void BotModel::addBot(QString path)
{
    QFileInfo file(path);

    if (!file.exists()) {
        QMessageBox::warning(0, tr("Unable to add bot"), tr("The path to the bot does not exist."));
        return;
    }
    const QString name = file.baseName();
    if (botNames().contains(name)) {
        qWarning() << "Bot" << name << "is already added";
        return;
    }

    Bot bot;
    bot.enabled = true;
    bot.path = path;
    bot.name = name;
    bot.wins = 0;
    bot.roundWins = 0;
    bot.totalScore = 0;
    bot.roundsPlayed = 0;

    beginInsertRows(QModelIndex(), m_bots.length(), m_bots.length() + 1);
    m_bots.append(bot);
    endInsertRows();

    save();
}

void BotModel::save() const
{
    QSettings settings;
    settings.setValue(BOTS_KEY, QVariant::fromValue(m_bots));
}

void BotModel::roundOver(QString name, bool isWinner, int roundWins, int score)
{
    for(int i=0; i<m_bots.length(); i++) {
        if (!m_bots[i].enabled) continue;
        if (m_bots[i].name != name) continue;

        if (isWinner) {
            m_bots[i].wins++;
        }
        m_bots[i].totalScore += score;
        m_bots[i].roundWins += roundWins;
        m_bots[i].roundsPlayed++;


        save();
        return;
    }
    const QString warningMessage(tr("Unable to find '%1' in the list of bots, please adjust score manually (winner?: %2, round wins: %3, score: %4").arg(name).arg(isWinner).arg(roundWins).arg(score));
    qWarning() << warningMessage;
    QMessageBox::warning(0, tr("Unable to find winner!"), warningMessage);
}

int BotModel::enabledPlayers()
{
    if (m_tournamentMode) {
        return TournamentController::instance()->getNextCompetitors().count();
    } else {
        int ret = 0;
        foreach(const Bot &bot, m_bots) {
            if (bot.enabled) {
                ret++;
            }
        }
        return ret;
    }
}

bool compareBots(Bot *a, const Bot *b)
{
    if (a->wins != b->wins) {
        return a->wins > b->wins;
    } else if (a->roundWins != b->roundWins) {
        return a->roundWins > b->roundWins;
    } else {
        return a->totalScore > b->totalScore;
    }
}

QStringList BotModel::topPlayers()
{
    QList<Bot*> bots;

    for (int i=0; i<m_bots.count(); i++) {
        bots.append(&m_bots[i]);
    }

    std::sort(bots.begin(), bots.end(), compareBots);

    QStringList names;
    for (int i=0; i<qMin(bots.count(), 3); i++) {
        names.append(bots[i]->name + " (" + QString::number(bots[i]->wins) + ")");
    }
    return names;
}

void BotModel::resetBots()
{
    beginResetModel();
    for (int i=0; i<m_bots.count(); i++) {
        m_bots[i].roundsPlayed = 0;
        m_bots[i].roundWins = 0;
        m_bots[i].totalScore = 0;
        m_bots[i].wins = 0;
    }
    endResetModel();

    save();
}

QStringList BotModel::botNames() const
{
    QStringList ret;
    for (const Bot &bot : m_bots) {
        if (!bot.enabled) {
            continue;
        }
        ret << bot.name;
    }
    return ret;
}

int BotModel::botWins(const QString &name) const
{
    for (const Bot &bot : m_bots) {
        if (bot.name == name) {
            return bot.wins;
        }
    }

    qWarning() << "Failed to find bot" << name;
    return -1;
}

bool BotModel::botIsValid(const QString botName)
{
    for (const Bot &bot : m_bots) {
        if (bot.name == botName) {
            // maybe check moar?
            return true;
        }
    }
    return false;
}

QStringList BotModel::enabledBotPaths() const
{
    QStringList botsToPlay;
    if (m_tournamentMode) {
        botsToPlay = TournamentController::instance()->getNextCompetitors();
    }

    QStringList paths;
    for (const Bot &bot : m_bots) {
        if (!bot.enabled) {
            continue;
        }
        if (m_tournamentMode && !botsToPlay.contains(bot.name)) {
            continue;
        }

        paths.append(bot.path);
    }
    return paths;
}

QDataStream &operator<<(QDataStream &out, const Bot &bot)
{
    out << bot.enabled << bot.name << bot.path << bot.wins << bot.roundWins << bot.totalScore << bot.roundsPlayed;
    return out;
}


QDataStream &operator>>(QDataStream &in, Bot &bot)
{
    in >> bot.enabled >> bot.name >> bot.path >> bot.wins >> bot.roundWins >> bot.totalScore >> bot.roundsPlayed;
    return in;
}
