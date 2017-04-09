#include "tournamentcontroller.h"
#include "botmodel.h"

#include <QDebug>
#include <QSettings>

QQmlListProperty<Round> TournamentController::winnerRounds()
{
    return QQmlListProperty<Round>(this, nullptr, &TournamentController::countRounds, &TournamentController::round);
}

void TournamentController::matchCompleted(const QMap<QString, int> &results)
{
    Round *round = currentRound();
    if (!round) {
        qWarning() << "Failed to get current round";
        return;
    }

    Match *currentMatch = round->currentMatch();
    if (!currentMatch){
        qWarning() << "Failed to get current match!";
        return;
    }

    for (const QString &name : results.keys()) {
        currentMatch->setResult(name, results[name]);
    }

    store();
}

void TournamentController::initializeMatches()
{
    const int playerCount = BotModel::instance()->rowCount();

    bool oddNumber = (playerCount % 2);

    const int initialMatches = playerCount / 2;
    Round *round1 = new Round("Round " + QString::number(1), this);

    int player = 0;
    for (int matchNumber = 0; matchNumber < initialMatches; matchNumber++) {
        Match *match = new Match(QString::number(matchNumber), this);
        Competitor *a = new Competitor(BotModel::instance()->botName(player++), this);
        Competitor *b = new Competitor(BotModel::instance()->botName(player++), this);
        match->addCompetitor(a);
        match->addCompetitor(b);
        round1->addMatch(match);
    }
    m_winnerBracket.append(round1);

    if (oddNumber) {
        qWarning() << "Odd number of players";

        Match *match = new Match(QString::number(initialMatches + 1), this);
        match->addCompetitor(new Competitor(BotModel::instance()->botName(player), this));
        Round *round2 = new Round("Round " + QString::number(2), this);
        round2->addMatch(match);
        m_winnerBracket.append(round2);
    }

    store();

    emit winnerRoundsChanged();
}

Round *TournamentController::currentRound() const
{
    for (Round *round : m_winnerBracket) {
        if (round->currentMatch()) {
            return round;
        }
    }

    for (Round *round : m_loserBracket) {
        if (round->currentMatch()) {
            return round;
        }
    }

    return nullptr;
}

QStringList TournamentController::getNextCompetitors() const
{
    const Round *nextRound = currentRound();
    if (!nextRound) {
        qWarning() << "No rounds left";
        return QStringList();
    }
    const Match *nextMatch = nextRound->currentMatch();
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
    for (const QString &roundName : settings.childGroups()) {
        Round *round = new Round(roundName, this);
        round->load(&settings);
        m_winnerBracket.append(round);
    }
    settings.endGroup();
    settings.beginGroup("Loser rounds");
    for (const QString &roundName : settings.childGroups()) {
        Round *round = new Round(roundName, this);
        round->load(&settings);
        m_loserBracket.append(round);
    }
}

void TournamentController::store() const
{
    QSettings settings;
    settings.beginGroup("Tournament");
    settings.beginGroup("Winner rounds");
    for (Round *round : m_winnerBracket) {
        round->store(&settings);
    }
    settings.endGroup();
    settings.beginGroup("Loser rounds");
    for (Round *round : m_loserBracket) {
        round->store(&settings);
    }
}

int TournamentController::countRounds(QQmlListProperty<Round> *list)
{
    TournamentController *me = qobject_cast<TournamentController*>(list->object);
    if (!me) {
        qWarning() << "Invalid object" << list->object;
        return 0;
    }

    return me->m_winnerBracket.count();
}

Round *TournamentController::round(QQmlListProperty<Round> *list, int num)
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



QQmlListProperty<Match> Round::matches()
{
    return QQmlListProperty<Match>(this, nullptr, &Round::countMatches, &Round::match);
}

Match *Round::currentMatch() const
{
    for (Match *match : m_matches) {
        if (!match->isDone()) {
            return match;
        }
    }

    return nullptr;
}

void Round::store(QSettings *settings) const
{
    settings->beginGroup(m_name);
    for (const Match *match : m_matches) {
        match->store(settings);
    }
    settings->endGroup();
}

void Round::load(QSettings *settings)
{
    settings->beginGroup(m_name);
    for (const QString &matchName :  settings->childGroups()) {
        Match *match = new Match(matchName, this);
        match->load(settings);
        m_matches.append(match);

    }
    settings->endGroup();
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



QQmlListProperty<Competitor> Match::competitors()
{
    return QQmlListProperty<Competitor>(this, nullptr, &Match::countCompetitors, &Match::competitor);
}

Competitor *Match::winner() const
{
    return *std::max_element(m_competitors.begin(), m_competitors.end(), [](const Competitor *a, const Competitor *b) {
        return a->score() > b->score();
    });
}

Competitor *Match::loser() const
{
    return *std::min_element(m_competitors.begin(), m_competitors.end(), [](const Competitor *a, const Competitor *b) {
        return a->score() > b->score();
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

void Match::setResult(const QString &name, int score)
{
    for (Competitor *competitor : m_competitors) {
        if (competitor->name() == name) {
            competitor->setScore(score);
            return;
        }
    }

    Competitor *w = winner();
    Competitor *l = loser();
    if (!w || !l) {
        return;
    }
    if (w->score() > l->score()) {
        w->setWinner(true);
        l->setWinner(false);
    } else {
        w->setWinner(false);
        l->setWinner(false);
    }
}

void Match::store(QSettings *settings) const
{
    settings->beginGroup(m_id);
    for (const Competitor *competitor : m_competitors) {
        competitor->store(settings);
    }
    settings->endGroup();
}

void Match::load(QSettings *settings)
{
    settings->beginGroup(m_id);
    for (const QString &competitorName : settings->childGroups()) {
        Competitor *competitor = new Competitor(competitorName, this);
        competitor->load(settings);
        m_competitors.append(competitor);
    }
    settings->endGroup();
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
    settings->beginGroup(m_name);
    settings->setValue("score", m_score);
    settings->setValue("winner", m_winner);
    settings->setValue("done", m_done);
    settings->endGroup();
}

void Competitor::load(QSettings *settings)
{
    settings->beginGroup(m_name);
    m_score = settings->value("score", 0).toInt();
    m_winner = settings->value("winner", false).toBool();
    m_done = settings->value("done", false).toBool();
    settings->endGroup();
}
