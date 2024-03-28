#ifndef EMAILDETAILWINDOW_H
#define EMAILDETAILWINDOW_H

#include <QDialog>

namespace Ui {
class EmailDetailWindow;
}

class EmailDetailWindow : public QDialog
{QString id,to,from,text,attachment,title;
    Q_OBJECT

public:
    explicit EmailDetailWindow(QWidget *parent = nullptr);
    ~EmailDetailWindow();
    void GetID(const QString &x);
    void openEmail();
    void processServerResponse(const QString& response);
    void showEvent(QShowEvent *event);
private slots:
    void on_downloadButton_clicked();

    void on_pushButton_clicked();

    void on_forwardButton_clicked();

private:
    Ui::EmailDetailWindow *ui;
};

#endif // EMAILDETAILWINDOW_H
