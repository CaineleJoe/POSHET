#include "secondwindow.h"
#include "ui_secondwindow.h"
#include"composewindow.h"
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
#include "emaildetailwindow.h"
#include <vector>
#include <algorithm>
using namespace std;
SecondWindow::SecondWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SecondWindow)
{
    ui->setupUi(this);
}

SecondWindow::~SecondWindow()
{
    delete ui;
}

void SecondWindow::on_button_compose_clicked()
{

    ComposeWindow composewindow;
    composewindow.setModal(true);
   composewindow.exec();
}


void SecondWindow::on_button_received_clicked()
{
        const string request = "LIST_RECV 1";
    write(sd, request.c_str(), request.length());

        char buffer[20000];
        ssize_t bytesReceived = read(sd, buffer, sizeof(buffer) - 1);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
        }            QString serverResponse = QString::fromUtf8(buffer);

    processServerResponse(serverResponse,false);
}



void SecondWindow::on_pushButton_clicked()
{
    QListWidgetItem *itm = ui->listWidget->currentItem();
    if (itm) {
        QString id = itm->text().split(" ").at(1);
        EmailDetailWindow emailwindow;
        emailwindow.GetID(id);
        emailwindow.setModal(true);
        emailwindow.exec();
    }

}


void SecondWindow::processServerResponse(const QString& response, bool isSent)
{
    ui->listWidget->clear();

    QStringList emails = response.split("\n\n", Qt::SkipEmptyParts);
    for (const QString& email : emails) {
        QStringList details = email.split("\n");
        qDebug() << "Email details:" << details;

        if (details.size() > 2) {
            QString id = details.at(0).split(": ").at(1);
            QString title = details.at(1).split(": ").at(1);
            QString from_or_to = details.at(2).split(": ").at(1);

            QString itemText = QString("ID: %1 Title: %2 %3: %4")
                                   .arg(id)
                                   .arg(title)
                                   .arg(isSent ? "To" : "From")
                                   .arg(from_or_to);

        QListWidgetItem* item = new QListWidgetItem(itemText);
        ui->listWidget->addItem(item);
    } else {
        qDebug() << "Email format is incorrect, skipping";
    }
}
}
void SecondWindow::on_button_sent_clicked()
{
    const string request = "LIST_SENT 1";
    write(sd, request.c_str(), request.length());

    char buffer[20000];
    ssize_t bytesReceived = read(sd, buffer, sizeof(buffer) - 1);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
    }            QString serverResponse = QString::fromUtf8(buffer);

    processServerResponse(serverResponse,true);
}


void SecondWindow::on_button_logout_clicked()
{
    string logoutCommand = "LOGOUT";
    write(sd, logoutCommand.c_str(), logoutCommand.length());
    setFlag(true);
        this->close();
}


void SecondWindow::on_pushButton_2_clicked()//newest button
{
    std::vector<std::pair<int, QString>> items;

    for(int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        QString itemText = item->text();
        QStringList parts = itemText.split(" ");
        int id = parts.at(1).toInt();
        items.push_back(std::make_pair(id, itemText));
    }

    std::sort(items.begin(), items.end(), [](const auto& a,const auto& b) {
        return a.first > b.first;
    });

    ui->listWidget->clear();
    for(const auto& item : items) {
        ui->listWidget->addItem(new QListWidgetItem(item.second));
    }

}

void SecondWindow::on_pushButton_3_clicked()//oldest button
{    std::vector<std::pair<int, QString>> items;

    for(int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        QString itemText = item->text();
        QStringList parts = itemText.split(" ");
        int id = parts.at(1).toInt();
        items.push_back(std::make_pair(id, itemText));
    }

    std::sort(items.begin(), items.end(), [](const auto& a,const auto& b) {
        return a.first < b.first;
    });

    ui->listWidget->clear();
    for(const auto& item : items) {
        ui->listWidget->addItem(new QListWidgetItem(item.second));
    }

}


void SecondWindow::on_pushButton_4_clicked()//search button
{
    QString searchText = ui->lineEdit->text().toLower();

    for(int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        item->setHidden(!item->text().toLower().contains(searchText));
    }
}

