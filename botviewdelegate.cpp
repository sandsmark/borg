#include "botviewdelegate.h"
#include "botmodel.h"
#include <QDebug>
#include <QComboBox>
#include <patheditor.h>
#include <QLineEdit>

BotViewDelegate::BotViewDelegate()
{

}

BotViewDelegate::~BotViewDelegate()
{

}

QWidget *BotViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    switch(index.column()) {
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

    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        lineEdit->setText(index.data().toString());
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

QSize BotViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (index.column() == BotModel::Path) {
        size.setWidth(300);
    }
    return size;
}
