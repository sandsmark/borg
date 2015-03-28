#ifndef PATHEDITOR_H
#define PATHEDITOR_H

#include <QWidget>
class QPushButton;
class QLineEdit;
class QFileDialog;


class PathEditor : public QWidget
{
    Q_OBJECT
public:
    explicit PathEditor(QWidget *parent = 0);
    ~PathEditor();

    QString path();

signals:
    void pathChanged(QString newPath);

public slots:
    void setPath(const QString &path);

private:
    QPushButton *m_button;
    QLineEdit *m_label;
    QFileDialog *m_dialog;
};

#endif // PATHEDITOR_H
