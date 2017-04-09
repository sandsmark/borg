#ifndef TOURNAMENTCONTROLLER_H
#define TOURNAMENTCONTROLLER_H

#include <QObject>
#include <QQmlListProperty>

class BotModel;

class Competitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(bool winner MEMBER m_winner NOTIFY winnerChanged)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)

public:
    Competitor(QString name, QObject *parent);

    int score() const { return m_score; }
    void setScore(int score) { m_done = true; m_score = score; emit scoreChanged(); }
    void setWinner(bool winner) { m_winner = winner; emit winnerChanged(); }
    bool done() const { return m_done; }
    QString name() const { return m_name; }

signals:
    void scoreChanged();
    void winnerChanged();

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
    QQmlListProperty<Competitor> competitors();

    const QList<const Competitor *> winners() const;
    QStringList competitorNames() const;
    void addCompetitor(Competitor *competitor);
    bool isDone() const;

    void setResult(const QString &name, int score);

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
    QQmlListProperty<Match> matches();

    void addMatch(Match *match) { m_matches.append(match); emit matchesChanged(); }
    Match *match(int match) const { if (match >= m_matches.count()) { return nullptr; } return m_matches[match]; }

    Match *currentMatch() const;

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
    Q_PROPERTY(QQmlListProperty<Round> rounds READ winnerRounds NOTIFY winnerRoundsChanged)

public:
    static TournamentController *instance() { static TournamentController me; return &me; }

    QQmlListProperty<Round> winnerRounds();

    void matchCompleted(const QMap<QString, int> &results);

    void initializeMatches();

    Round *currentRound() const;

    QStringList getNextCompetitors() const;

signals:
    void winnerRoundsChanged();

public slots:

private:
    TournamentController() { initializeMatches(); }

    static int countRounds(QQmlListProperty<Round> *list);
    static Round *round(QQmlListProperty<Round> *list, int num);

    QList<Round*> m_winnerRounds;
    QList<Round*> m_loserRounds;
};

#endif // TOURNAMENTCONTROLLER_H
