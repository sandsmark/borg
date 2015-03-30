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
    QString runtime;
    QString path;
    int wins;
    QSharedPointer<QProcess> process;
    QSharedPointer<QFile> logfile;
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
        Runtime,
        Path,
        Wins,
        Running
    };

    BotModel(QObject *parent);
    ~BotModel();

    int rowCount(const QModelIndex &) const { return m_bots.length(); }
    int columnCount(const QModelIndex &) const { return 6; }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void removeRow(int row);
    void addBot(QString path);

    void save();

    static QHash<QString, QString> runtimes();

    void giveWin(QString name);

public slots:
    void launchBots();
    void killBots();

private slots:
    void storeOutput();
    void botStateChanged();

private:
    void initializeBotProcess(Bot *bot);

    QList<Bot> m_bots;
};

#endif // BOTMODEL_H
