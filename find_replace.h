#ifndef FIND_REPLACE_H
#define FIND_REPLACE_H

#include <QDialog>

namespace Ui {
    class Find_Replace;
}

class Find_Replace : public QDialog
{
    Q_OBJECT
public:
    Find_Replace(QWidget *parent = nullptr);
    ~Find_Replace();
    Ui::Find_Replace *ui;

protected:
    void changeEvent(QEvent *event);

private slots:
    void button_Enabled();
};

#endif // FIND_REPLACE_H
