#include "tournamentcontroller.h"
#include "botmodel.h"

#include <QDebug>
#include <QSettings>
#include <qmath.h>

QQmlListProperty<Round> TournamentController::winnerRounds()
{
    return QQmlListProperty<Round>(this, nullptr, &TournamentController::countWinnersRounds, &TournamentController::winnersRound);
}

QQmlListProperty<Round> TournamentController::losersRounds()
{
    return QQmlListProperty<Round>(this, nullptr, &TournamentController::countLosersRounds, &TournamentController::losersRound);
}

void TournamentController::onMatchCompleted(const QMap<QString, int> &results)
{
    Round *round = nextUnplayedRound();
    if (!round) {
        qWarning() << "Failed to get current round";
        return;
    }

    Match *currentMatch = round->firstUnplayedMatch();
    if (!currentMatch){
        qWarning() << "Failed to get current match!";
        return;
    }

    for (const QString &name : results.keys()) {
        currentMatch->setResult(name, results[name]);
    }
    Competitor *winner = currentMatch->winner();
    Competitor *loser = currentMatch->loser();

    winner->setWinner(true);
    loser->setWinner(false);

    Round *nextWinnerRound = nextOpenWinnersRound();
    if (nextWinnerRound) {
        nextWinnerRound->firstOpenMatch()->addCompetitor(new Competitor(winner->name(), nextWinnerRound));
    }

    Round *nextLoserRound = nextOpenLosersRound();
    if (nextLoserRound) {
        nextLoserRound->firstOpenMatch()->addCompetitor(new Competitor(loser->name(), nextLoserRound));
    }

    store();
}

static inline bool isPowerOfTwo(int number)
{
    return (number & (number - 1)) == 0;
}

void TournamentController::initializeMatches()
{
    for (Round *round : m_winnerBracket) {
        round->clear();
        round->deleteLater();
    }
    m_winnerBracket.clear();
    for (Round *round : m_loserBracket) {
        round->clear();
        round->deleteLater();
    }
    m_loserBracket.clear();
    QSettings settings;
    settings.remove("Tournament");

    int roundNumber = 1;
    int matchNumber = 1;
    QStringList players = BotModel::instance()->botNames();
    const int playerCount = players.count();
    int byes = 0;
    // If player count is not a power of two, we need to add byes
    if (!isPowerOfTwo(playerCount)) {
        byes = qNextPowerOfTwo(playerCount) - playerCount;
    }
    Round *winnerRound = new Round("Round " + QString::number(roundNumber++), this);
    const int initialMatches = (playerCount - byes) / 2;
//    int accumulatedLosers = 0;
    for (int i=0; i<initialMatches; i++) {
            Match *match = new Match(QString::number(matchNumber++), winnerRound);

            if (!players.isEmpty()) {
                match->addCompetitor(new Competitor(players.takeFirst(), match));
            }
            if (!players.isEmpty()) {
                match->addCompetitor(new Competitor(players.takeFirst(), match));
            }
            winnerRound->addMatch(match);

//            accumulatedLosers++;
    }
    m_winnerBracket.append(winnerRound);

    qDebug() << "byes" << "power of two winners?" << isPowerOfTwo(initialMatches);
//    qDebug() << "losers" << accumulatedLosers;

    if (byes) {
        winnerRound = new Round("Round " + QString::number(roundNumber++), this);
        const int playersLeft = players.count();
        const int singles = qNextPowerOfTwo(playersLeft) - playersLeft;

        for (int i=0; i<singles; i++) {
            Match *match = new Match(QString::number(matchNumber++), winnerRound);
            if (!players.isEmpty()) {
                match->addCompetitor(new Competitor(players.takeFirst(), match));
            }
            winnerRound->addMatch(match);
//            accumulatedLosers++;
        }

        const int overflow = playersLeft - singles;
        for (int i=0; i<overflow/2; i++) {
            Match *match = new Match(QString::number(matchNumber++), winnerRound);
            if (!players.isEmpty()) {
                match->addCompetitor(new Competitor(players.takeFirst(), match));
            }
            if (!players.isEmpty()) {
                match->addCompetitor(new Competitor(players.takeFirst(), match));
            }
            winnerRound->addMatch(match);
//            accumulatedLosers++;
        }
        const int missingMatches = (initialMatches - singles) / 2;
        for (int i=0; i<missingMatches; i++) {
            Match *match = new Match(QString::number(matchNumber++), winnerRound);
            winnerRound->addMatch(match);
//            accumulatedLosers++;
        }
        m_winnerBracket.append(winnerRound);
    }


    if (!players.isEmpty()) {
        qWarning() << "players not assigned!" << players;
    }

    // Fill up the rest of the winner rounds
    while (m_winnerBracket.last()->matchCount() > 1) {
//        if (m_winnerBracket.last()->matchCount() == 2) {
//            winnerRound = new Round("Final", this);
//        } else {
            winnerRound = new Round("Round " + QString::number(roundNumber++), this);
//        }
        for (int i=0; i < m_winnerBracket.last()->matchCount()/2; i++) {
            winnerRound->addMatch(new Match(QString::number(matchNumber++), winnerRound));
        }
        m_winnerBracket.append(winnerRound);
    }

    // Fill up the losers bracket
    int accumulatedLosers = 0;
    matchNumber = 1;
    for (int i=0; i<m_winnerBracket.count(); i++) {
        Round *loserRound = new Round("Loser round " + QString::number(i + 1), this);
        accumulatedLosers += m_winnerBracket[i]->matchCount();
        while (accumulatedLosers >= 2) {
            Match *loserMatch = new Match(QString::number(matchNumber++), loserRound);
            loserRound->addMatch(loserMatch);
            accumulatedLosers -= 2;
        }
        accumulatedLosers += loserRound->matchCount();
        m_loserBracket.append(loserRound);
    }

    // Fill up the rest of the loser rounds
    while (accumulatedLosers >= 2) {
        Round *loserRound = new Round("Loser round " + QString::number(roundNumber++), this);
        while (accumulatedLosers >= 2) {
            loserRound->addMatch(new Match(QString::number(matchNumber++), loserRound));
            accumulatedLosers -= 2;
        }
        accumulatedLosers += loserRound->matchCount();
        m_loserBracket.append(loserRound);
    }

    // Add final
    Round *finalRound = new Round("Final", this);
    finalRound->addMatch(new Match(QString::number(matchNumber++), finalRound));
    m_winnerBracket.append(finalRound);

    emit winnerRoundsChanged();
    emit losersRoundsChanged();

    store();
}

Round *TournamentController::nextUnplayedRound() const
{
    for (Round *round : m_loserBracket) {
        if (!round->firstOpenMatch() && round->firstUnplayedMatch()) {
            return round;
        }
    }

    for (Round *round : m_winnerBracket) {
        if (!round->firstOpenMatch() && round->firstUnplayedMatch()) {
            return round;
        }
    }

    return nullptr;
}

Round *TournamentController::nextOpenLosersRound() const
{
    for (Round *round : m_loserBracket) {
        if (round->firstOpenMatch()) {
            return round;
        }
    }

    return nullptr;
}

Round *TournamentController::nextOpenWinnersRound() const
{
    for (Round *round : m_winnerBracket) {
        if (round->firstOpenMatch()) {
            return round;
        }
    }

    return nullptr;
}

QStringList TournamentController::getNextCompetitors() const
{
    Round *nextRound = nextUnplayedRound();
    if (!nextRound) {
        qWarning() << "No rounds left";
        return QStringList();
    }

    const Match *nextMatch = nextRound->firstUnplayedMatch();
    if (!nextMatch) {
        qWarning() << "No matches left";
        return QStringList();
    }

    return nextMatch->competitorNames();
}

void TournamentController::load()
{
    QSettings settings;
    settings.beginGroup("Tournament");
    settings.beginGroup("Winner rounds");
    for (const QString &groupName : settings.childGroups()) {
        settings.beginGroup(groupName);
        Round *round = new Round(settings.value("name").toString(), this);
        round->load(&settings);
        m_winnerBracket.append(round);
        settings.endGroup();
    }
    settings.endGroup();
    settings.beginGroup("Loser rounds");
    for (const QString &groupName : settings.childGroups()) {
        settings.beginGroup(groupName);
        Round *round = new Round(settings.value("name").toString(), this);
        round->load(&settings);
        m_loserBracket.append(round);
        settings.endGroup();
    }
}

void TournamentController::store() const
{
    QSettings settings;
    settings.beginGroup("Tournament");
    settings.beginGroup("Winner rounds");
    for (int i=0; i<m_winnerBracket.count(); i++) {
        settings.beginGroup("round " + QString::number(i, 16));
        m_winnerBracket[i]->store(&settings);
        settings.endGroup();
    }
    settings.endGroup();
    settings.beginGroup("Loser rounds");
    for (int i=0; i<m_loserBracket.count(); i++) {
        settings.beginGroup("round " + QString::number(i, 16));
        m_loserBracket[i]->store(&settings);
        settings.endGroup();
    }
}

int TournamentController::countWinnersRounds(QQmlListProperty<Round> *list)
{
    TournamentController *me = qobject_cast<TournamentController*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return 0;
    }

    return me->m_winnerBracket.count();
}

int TournamentController::countLosersRounds(QQmlListProperty<Round> *list)
{
    TournamentController *me = qobject_cast<TournamentController*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return 0;
    }

    return me->m_loserBracket.count();
}

Round *TournamentController::winnersRound(QQmlListProperty<Round> *list, int num)
{
    TournamentController *me = qobject_cast<TournamentController*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return nullptr;
    }

    if (num >= me->m_winnerBracket.count()) {
        qWarning() << "Invalid round number" << num;
        return nullptr;
    }

    return me->m_winnerBracket[num];
}

Round *TournamentController::losersRound(QQmlListProperty<Round> *list, int num)
{

    TournamentController *me = qobject_cast<TournamentController*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return nullptr;
    }

    if (num >= me->m_loserBracket.count()) {
        qWarning() << "Invalid round number" << num;
        return nullptr;
    }

    return me->m_loserBracket[num];
}



Round::~Round()
{
}

QQmlListProperty<Match> Round::matches()
{
    return QQmlListProperty<Match>(this, nullptr, &Round::countMatches, &Round::match);
}

void Round::clear()
{
    for (Match *match : m_matches) {
        match->clear();
    }
    m_matches.clear();
    emit matchesChanged();
}

Match *Round::firstUnplayedMatch() const
{
    for (Match *match : m_matches) {
        if (!match->isDone()) {
            return match;
        }
    }

    return nullptr;
}

Match *Round::firstOpenMatch() const
{
    for (Match *match : m_matches) {
        if (!match->isReady()) {
            return match;
        }
    }

    return nullptr;
}

void Round::store(QSettings *settings) const
{
    settings->setValue("name", m_name);
    for (int i=0; i<m_matches.count(); i++) {
        settings->beginGroup("match" + QString::number(i, 16));
        m_matches[i]->store(settings);
        settings->endGroup();
    }
}

void Round::load(QSettings *settings)
{
    for (const QString &groupName :  settings->childGroups()) {
        settings->beginGroup(groupName);
        Match *match = new Match(settings->value("id").toString(), this);
        match->load(settings);
        m_matches.append(match);
        settings->endGroup();
    }
}

int Round::countMatches(QQmlListProperty<Match> *list)
{
    Round *me = qobject_cast<Round*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return 0;
    }

    return me->m_matches.count();
}

Match *Round::match(QQmlListProperty<Match> *list, int num)
{
    Round *me = qobject_cast<Round*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return nullptr;
    }
    if (num >= me->m_matches.count()) {
        qWarning() << "Invalid match number" << num;
        return nullptr;
    }

    return me->m_matches[num];
}



void Match::clear()
{
    m_competitors.clear();
    emit competitorsChanged();
}

QQmlListProperty<Competitor> Match::competitors()
{
    return QQmlListProperty<Competitor>(this, nullptr, &Match::countCompetitors, &Match::competitor);
}

Competitor *Match::winner() const
{
    return *std::max_element(m_competitors.begin(), m_competitors.end(), [](const Competitor *a, const Competitor *b) {
        return a->score() < b->score();
    });
}

Competitor *Match::loser() const
{
    return *std::min_element(m_competitors.begin(), m_competitors.end(), [](const Competitor *a, const Competitor *b) {
        return a->score() < b->score();
    });
}

QStringList Match::competitorNames() const
{
    QStringList names;
    for (const Competitor *competitor : m_competitors) {
        names.append(competitor->name());
    }
    return names;
}

void Match::addCompetitor(Competitor *competitor)
{
    m_competitors.append(competitor);
    emit competitorsChanged();
}

bool Match::isDone() const
{
    bool done = true;
    for (const Competitor *competitor : m_competitors) {
        done = done && competitor->done();
    }
    return done;
}

bool Match::isReady() const
{
    return m_competitors.count() > 1;
}

bool Match::setResult(const QString &name, int score)
{
    for (Competitor *competitor : m_competitors) {
        if (competitor->name() == name) {
            competitor->setScore(score);
            return true;
        }
    }

    return false;
}

void Match::store(QSettings *settings) const
{
    settings->setValue("id", m_id);
    for (int i=0; i<m_competitors.count(); i++) {
        settings->beginGroup("competitor" + QString::number(i, 16));
        m_competitors[i]->store(settings);
        settings->endGroup();
    }
}

void Match::load(QSettings *settings)
{
    m_id = settings->value("id").toString();
    for (const QString &groupName : settings->childGroups()) {
        settings->beginGroup(groupName);
        Competitor *competitor = new Competitor(settings->value("name").toString(), this);
        competitor->load(settings);
        m_competitors.append(competitor);
        settings->endGroup();
    }
}

int Match::countCompetitors(QQmlListProperty<Competitor> *list)
{
    Match *me = qobject_cast<Match*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return 0;
    }

    return me->m_competitors.count();
}

Competitor *Match::competitor(QQmlListProperty<Competitor> *list, int num)
{
    Match *me = qobject_cast<Match*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return nullptr;
    }
    if (num >= me->m_competitors.count()) {
        qWarning() << "Invalid match number" << num;
        return nullptr;
    }

    return me->m_competitors[num];

}

Competitor::Competitor(QString name, QObject *parent) : QObject(parent),
    m_name(name),
    m_score(0),
    m_winner(false),
    m_done(false)
{
}

void Competitor::store(QSettings *settings) const
{
    settings->setValue("name", m_name);
    settings->setValue("score", m_score);
    settings->setValue("winner", m_winner);
    settings->setValue("done", m_done);
}

void Competitor::load(QSettings *settings)
{
    m_score = settings->value("score").toInt();
    m_winner = settings->value("winner").toBool();
    m_done = settings->value("done").toBool();
}

bool Competitor::isValid() const
{
    return BotModel::instance()->botIsValid(m_name);
}
