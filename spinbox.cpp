#include "spinbox.h"
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>

SpinBox::SpinBox(QString text, QWidget *parent) : QWidget(parent)
{
    setLayout(new QHBoxLayout);
    layout()->addWidget(new QLabel(text));
    m_spinbox = new QSpinBox;
    layout()->addWidget(new QSpinBox);
}
