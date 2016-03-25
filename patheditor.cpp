#include "patheditor.h"
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QFileDialog>

PathEditor::PathEditor(QWidget *parent) : QWidget(parent),
    m_button(new QPushButton),
    m_label(new QLineEdit),
    m_dialog(new QFileDialog)
{
    m_button->setText("Browse...");
    m_label->setReadOnly(true);

    setLayout(new QHBoxLayout);
    layout()->addWidget(m_label);
    layout()->addWidget(m_button);

    connect(m_button, SIGNAL(clicked()), m_dialog, SLOT(show()));
    connect(m_dialog, SIGNAL(fileSelected(QString)), SLOT(setPath(QString)));
    connect(m_dialog, SIGNAL(fileSelected(QString)), SIGNAL(pathChanged(QString)));
}

PathEditor::~PathEditor()
{

}

QString PathEditor::path()
{
    return m_label->text();
}

void PathEditor::setPath(const QString &path)
{
    QFileInfo file(path);
    m_dialog->setDirectory(file.path());
    if (!file.suffix().isEmpty()) {
//        m_dialog->setNameFilter("*.*");
    }
    m_label->setText(path);
}

