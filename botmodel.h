#ifndef BOTMODEL_H
#define BOTMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QList>

struct Bot {
    bool enabled;
    QString name;
    QString runtime;
    QString path;
    int wins;
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
        Wins
    };

    BotModel(QObject *parent);
    ~BotModel();

    int rowCount(const QModelIndex &) const { return m_bots.length(); }
    int columnCount(const QModelIndex &) const { return 5; }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    QList<Bot> m_bots;
};

#endif // BOTMODEL_H
