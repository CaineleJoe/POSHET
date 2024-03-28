#ifndef COMPOSEWINDOW_H
#define COMPOSEWINDOW_H

#include <QDialog>

namespace Ui {
class ComposeWindow;
}

class ComposeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ComposeWindow(QWidget *parent = nullptr);
    ~ComposeWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ComposeWindow *ui;
};

#endif // COMPOSEWINDOW_H
