#ifndef BOTMODEL_H
#define BOTMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QList>
#include <QProcess>
#include <QHash>

struct Bot {
    bool enabled;
    QString name;
    QString runtime;
    QString path;
    int wins;
    QProcess *process;
};

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
    void launchBots();
    void killBots();
    void addBot(QString path);

    void save();

    static QHash<QString, QString> runtimes();

private:
    QList<Bot> m_bots;
};

#endif // BOTMODEL_H
