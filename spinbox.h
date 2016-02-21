#ifndef SPINBOX_H
#define SPINBOX_H

#include <QWidget>
#include <QSpinBox>

class SpinBox : public QWidget
{
    Q_OBJECT
public:
    explicit SpinBox(QString text, QWidget *parent = 0);

    void setMinimum(int min) { m_spinbox->setMinimum(min); }
    void setMaximum(int max) { m_spinbox->setMaximum(max); }
    int value() { return m_spinbox->value(); }

signals:

public slots:
private:
    QSpinBox *m_spinbox;
};

#endif // SPINBOX_H
