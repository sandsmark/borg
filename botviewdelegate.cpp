#include "botviewdelegate.h"
#include "botmodel.h"
#include <QDebug>
#include <QComboBox>
#include <patheditor.h>

BotViewDelegate::BotViewDelegate()
{

}

BotViewDelegate::~BotViewDelegate()
{

}

QWidget *BotViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch(index.column()) {
    case BotModel::Runtime: {
        QComboBox *combobox = new QComboBox(parent);
        combobox->addItems(BotModel::runtimes().keys());
        return combobox;
    }
    case BotModel::Path:
        return new PathEditor(parent);
        break;
    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    return 0;
}

void BotViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    PathEditor *pathEditor = qobject_cast<PathEditor*>(editor);
    if (pathEditor) {
        pathEditor->setPath(index.data().toString());
        return;
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void BotViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

void BotViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    PathEditor *pathEditor = qobject_cast<PathEditor*>(editor);
    if (pathEditor) {
        model->setData(index, pathEditor->path());
        return;
    }

    QStyledItemDelegate::setModelData(editor, model, index);
}
