#ifndef BOTMODEL_H
#define BOTMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QList>
#include <QProcess>
#include <QHash>
#include <QFile>
#include <QSharedPointer>

struct Bot {
    bool enabled;
    QString name;
    QString path;
    int wins = -1;
    int roundWins = -1;
    int totalScore = -1;
    int roundsPlayed = -1;
};

QDataStream &operator<<(QDataStream &out, const Bot &bot);
QDataStream &operator>>(QDataStream &in, Bot &bot);
Q_DECLARE_METATYPE(Bot)

class BotModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        Enabled = 0,
        Name,
        Path,
        Wins,
        RoundWins,
        TotalScore,
        RoundsPlayed,
        ColumnCount
    };

    static BotModel *instance() { static BotModel me; return &me; }

    ~BotModel();

    int rowCount(const QModelIndex & = QModelIndex()) const override { return m_bots.length(); }
    int columnCount(const QModelIndex &) const override { return ColumnCount; }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void removeRow(int row);
    void addBot(QString path);

    void save() const;

    void roundOver(QString name, bool isWinner, int roundWins, int score);

    int enabledPlayers();

    QStringList topPlayers();

    void resetBots();

    QStringList botNames() const;
    int botWins(const QString &name) const;
    QString botName(int row) const { if (row < 0 || row >= rowCount()) return QString(); return m_bots[row].name; }
    bool botIsValid(const QString botName);

    QStringList enabledBotPaths() const;

    void setTournamentMode(bool enabled) { m_tournamentMode = enabled; }

private:
    BotModel();

    QList<Bot> m_bots;
    bool m_tournamentMode;
};

#endif // BOTMODEL_H
