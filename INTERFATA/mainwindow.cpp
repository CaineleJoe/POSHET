#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <fstream>
#include<libgen.h>
#include <global.h>
#include "secondwindow.h"
using namespace std;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{char msg[100];
    bzero(msg,100);
    QString qUsername = ui->form_user->text();
    QString qPassword = ui->form_password->text();

    string username = qUsername.toStdString();
    string password = qPassword.toStdString();


    string message = "LOGIN " + username + " " + password;
    write(sd, message.c_str(), message.length());



    if (read(sd, msg,sizeof(msg)) <= 0) {
        printf("Conexiunea cu serverul a fost inchisa sau a aparut o eroare.\n");

    }
    ui->label_status->setText(QString(msg));
    if (ui->label_status->text() == "+OK Autentificare reusita") {
        this->hide();
        SecondWindow secondwindow;
        secondwindow.setModal(true);
        secondwindow.exec();
        if(secondwindow.getFlag())
        {ui->form_user->clear();
        ui->form_password->clear();
        ui->label_status->clear();
        this->show();
        }
        else
            this->close();
    }else {
        ui->form_user->clear();
        ui->form_password->clear();
    }
}

