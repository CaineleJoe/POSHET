#include "emaildetailwindow.h"
#include "ui_emaildetailwindow.h"
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
#include <QLineEdit>
#include <QInputDialog>
#include "composewindow.h"
#include <QPlainTextEdit>
using namespace std;
EmailDetailWindow::EmailDetailWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EmailDetailWindow)
{
    ui->setupUi(this);
}

EmailDetailWindow::~EmailDetailWindow()
{
    delete ui;
}
 void EmailDetailWindow::GetID(const QString &x)
{id=x;

 }
void EmailDetailWindow::openEmail()
 {
    int emailId = id.toInt();
    const string request = "OPEN " + to_string(emailId);
    write(sd, request.c_str(), request.length());
    char buffer[4096];
    ssize_t bytesReceived = read(sd, buffer, sizeof(buffer) - 1);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        QString serverResponse = QString::fromUtf8(buffer);

        processServerResponse(serverResponse);
    } else {
        qDebug()<<"Error retrieving e-mail's data";
    }
}
void EmailDetailWindow::processServerResponse(const QString& response)
{    qDebug() << "Server response:" << response;
    QStringList details = response.split("\n");
    qDebug()<<details.size();
    if (details.size() >=5) {  // Verifica daca exista cel putin 6 linii (ID, Title, From, To, Text, Attachment)
        title = details.at(1).split(": ").at(1);
        from = details.at(2).split(": ").at(1);
        to = details.at(3).split(": ").at(1);
        text = details.at(4).split(": ").at(1);
        attachment = details.at(5).split(": ").at(1);

                ui->titleLabel->setText(title);
        ui->fromLabel->setText(from);
        ui->toLabel->setText(to);
        ui->textEdit->setText(text);
        ui->attachmentLabel->setText(attachment);
        if (!attachment.isEmpty()) {
            ui->downloadButton->setVisible(true);
        } else {
            ui->downloadButton->setVisible(false);
        }
    } else {
        qDebug()<<"Error in e-mail format";
    }

}
    void EmailDetailWindow::showEvent(QShowEvent *event)
    {
        QDialog::showEvent(event);
        openEmail();
    }

void EmailDetailWindow::on_downloadButton_clicked()
{   // Trimite comanda de download la server
    const string request = "DOWNLOAD " + attachment.toStdString();
    write(sd, request.c_str(), request.length());

    char file_name[100];
    strncpy(file_name, attachment.toStdString().c_str(), sizeof(file_name));
    char file_path[200];
    sprintf(file_path, "/home/kali/Downloads/%s", basename(file_name));


    FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        qDebug()<<"Error opening file for writing";
        return;
    }


        char buffer[1024];
    int bytes_read;
    bool end_of_file = false;
    while (!end_of_file && (bytes_read = recv(sd, buffer, sizeof(buffer), 0)) > 0) {
        if (bytes_read >= 3 && strncmp(buffer + bytes_read - 3, "EOF", 3) == 0) {
            // Scrie totul in fisier in afara de EOF
            fwrite(buffer, 1, bytes_read - 3, file);
            end_of_file = true;

            qDebug()<<"bytes final:"<<bytes_read-3;
            break;
        } else {
            fwrite(buffer, 1, bytes_read, file);
            qDebug()<<"bytes: "<<bytes_read;

        }
    }
    bool downloadSuccessful = false;
    if (file != NULL && end_of_file) {
        downloadSuccessful = true;
    }

    fclose(file);
    if (downloadSuccessful) {
        ui->downloadButton->setText("Succes!");
    } else {

        ui->downloadButton->setText("Failed.");
    }
}




void EmailDetailWindow::on_forwardButton_clicked()
{
    bool ok=false;
    QString newRecipient = QInputDialog::getText(this, "Forward Email", "Enter new recipient's email:", QLineEdit::Normal, QString(), &ok);

    if (ok && !newRecipient.isEmpty()) {
        QString newTitle = "[FORWARDED] " + title;
        QString newContent = text;
        QString forwardCommand = "COMPOSE -to " + newRecipient + " -title " + newTitle + " -text " + newContent;
        if (!attachment.isEmpty()) {
            forwardCommand += " -attachment " + attachment;
        }
        write(sd, forwardCommand.toStdString().c_str(), forwardCommand.length());

        ui->forwardButton->setText("Forwarded to: " + newRecipient);
    }
}


void EmailDetailWindow::on_pushButton_clicked()//reply button
{    bool ok=false;

    QString replyText = QInputDialog::getMultiLineText(this, "Reply", "Enter your reply", QString(), &ok);
    if (ok && !replyText.isEmpty()) {
        QString fullReplyText = "Replied to MAIL ID [" + id + "] " + replyText;
        QString replyCommand = "COMPOSE -to " + from + " -title " + "[Re] " + title + " -text " + fullReplyText;

        // Trimite comanda la server
        write(sd, replyCommand.toStdString().c_str(), replyCommand.length());
    ui->pushButton->setText("Reply Sent!");
    }
}

