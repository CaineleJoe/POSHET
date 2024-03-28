#ifndef SECONDWINDOW_H
#define SECONDWINDOW_H

#include <QDialog>

namespace Ui {
class SecondWindow;
}

class SecondWindow : public QDialog
{    char msg[2048];
    bool flag=false;
    Q_OBJECT

public:
    explicit SecondWindow(QWidget *parent = nullptr);
    void processServerResponse(const QString& response,bool isSent);
    void setFlag(bool x)
    {flag=x;
    }
    bool getFlag()
    {
        return flag;
    }
    ~SecondWindow();

private slots:
    void on_button_compose_clicked();

    void on_button_received_clicked();

    void on_pushButton_clicked();

    void on_button_sent_clicked();

    void on_button_logout_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::SecondWindow *ui;
};

#endif // SECONDWINDOW_H
