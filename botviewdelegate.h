#ifndef BOTVIEWDELEGATE_H
#define BOTVIEWDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class BotViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    BotViewDelegate();
    ~BotViewDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

};

#endif // BOTVIEWDELEGATE_H
