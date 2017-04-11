#ifndef TOURNAMENTCONTROLLER_H
#define TOURNAMENTCONTROLLER_H

#include <QObject>
#include <QQmlListProperty>
#include <QDebug>

class BotModel;
class QSettings;

class Competitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(bool winner MEMBER m_winner NOTIFY winnerChanged)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    Q_PROPERTY(bool done READ done NOTIFY doneChanged)

public:
    Competitor(QString name, QObject *parent);

    int score() const { return m_score; }
    void setScore(int score) { m_done = true; m_score = score; emit scoreChanged(); emit doneChanged(); }
    void setWinner(bool winner) { m_winner = winner; emit winnerChanged(); }
    bool done() const { return m_done; }
    QString name() const { return m_name; }

    void store(QSettings *settings) const;
    void load(QSettings *settings);

    bool isValid() const;

signals:
    void scoreChanged();
    void winnerChanged();
    void doneChanged();

private:

    QString m_name;
    int m_score;
    bool m_winner;
    bool m_done;
};

class Match : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_id CONSTANT)
    Q_PROPERTY(QQmlListProperty<Competitor> competitors READ competitors NOTIFY competitorsChanged)

public:
    Match(const QString id, QObject *parent) : QObject(parent), m_id(id) {}
    void clear();
    QQmlListProperty<Competitor> competitors();

    Competitor *winner() const;
    Competitor *loser() const;
    QStringList competitorNames() const;
    void addCompetitor(Competitor *competitor);
    bool isDone() const;
    bool isReady() const;

    bool setResult(const QString &name, int score);

    void store(QSettings *settings) const;
    void load(QSettings *settings);

signals:
    void competitorsChanged();

private:
    static int countCompetitors(QQmlListProperty<Competitor> *list);
    static Competitor *competitor(QQmlListProperty<Competitor> *list, int num);

    QString m_id;
    QList<Competitor*> m_competitors;
};

class Round : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(QQmlListProperty<Match> matches READ matches NOTIFY matchesChanged)

public:
    Round(const QString &name, QObject *parent) : QObject(parent), m_name(name) {}
    virtual ~Round();
    QQmlListProperty<Match> matches();

    void clear();

    void addMatch(Match *match) { m_matches.append(match); emit matchesChanged(); }
    Match *match(int match) const { if (match >= m_matches.count()) { return nullptr; } return m_matches[match]; }

    Match *firstUnplayedMatch() const;
    Match *firstOpenMatch() const;

    void store(QSettings *settings) const;
    void load(QSettings *settings);

    int matchCount() const { return m_matches.count(); }

signals:
    void matchesChanged();

private:
    static int countMatches(QQmlListProperty<Match> *list);
    static Match *match(QQmlListProperty<Match> *list, int num);

    QString m_name;
    QList<Match*> m_matches;
};

class TournamentController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Round> winnersRounds READ winnerRounds NOTIFY winnerRoundsChanged)
    Q_PROPERTY(QQmlListProperty<Round> losersRounds READ losersRounds NOTIFY losersRoundsChanged)

public:
    static TournamentController *instance() { static TournamentController me; return &me; }

    QQmlListProperty<Round> winnerRounds();
    QQmlListProperty<Round> losersRounds();

    void onMatchCompleted(const QMap<QString, int> &results);

    void initializeMatches();

    Round *nextUnplayedRound() const;

    Round *nextOpenLosersRound() const;
    Round *nextOpenWinnersRound() const;

    QStringList getNextCompetitors() const;

    void load();
    void store() const;

signals:
    void winnerRoundsChanged();
    void losersRoundsChanged();

public slots:

private:
    TournamentController() { load(); qDebug() << m_winnerBracket.count(); if (m_winnerBracket.isEmpty()) { initializeMatches(); } }

    static int countWinnersRounds(QQmlListProperty<Round> *list);
    static int countLosersRounds(QQmlListProperty<Round> *list);
    static Round *winnersRound(QQmlListProperty<Round> *list, int num);
    static Round *losersRound(QQmlListProperty<Round> *list, int num);

    QList<Round*> m_winnerBracket;
    QList<Round*> m_loserBracket;
};

#endif // TOURNAMENTCONTROLLER_H
