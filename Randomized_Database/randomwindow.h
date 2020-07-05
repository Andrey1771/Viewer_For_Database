#ifndef RANDOMWINDOW_H
#define RANDOMWINDOW_H

#include <QDialog>

namespace Ui {
class RandomWindow;
}

class RandomWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RandomWindow(QWidget *parent = nullptr);
    ~RandomWindow();

private:
    Ui::RandomWindow *ui;
};

#endif // RANDOMWINDOW_H
