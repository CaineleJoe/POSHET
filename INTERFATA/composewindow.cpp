#include "composewindow.h"
#include "ui_composewindow.h"
#include "global.h"
#include <QRegularExpression>
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

using namespace std;
ComposeWindow::ComposeWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ComposeWindow)
{
    ui->setupUi(this);
}

ComposeWindow::~ComposeWindow()
{
    delete ui;
}

void ComposeWindow::on_pushButton_clicked()
{ QString qTo = ui->line_to->text();
    QString qTitle = ui->line_title->text();
    QString qText = ui->box_text->toPlainText();
    QString qAttachment = ui->line_attachment->text();

    // Verifica daca campurile obligatorii sunt completate
    if (qTo.isEmpty() || qTitle.isEmpty() || qText.isEmpty()) {
        ui->label_eroare1->setText("Toate campurile cu \"*\" trebuie completate!");
        return;
    }

    // Verifica daca atasamentul corespunde tiparului cerut
    QRegularExpression regex("^/([^/ ]*/)*([^/ ]+\\.[^/ ]+)$");
    QRegularExpressionMatch match = regex.match(qAttachment);
    if (!qAttachment.isEmpty() && !match.hasMatch()) {
        ui->label_eroare2->setText("Atasamentul nu corespunde tiparului cerut: /path/to/file.extensie");
        return;
    }

    // Conditie pentru a verifica daca exista fisierul
    string attachmentPath = qAttachment.toStdString();
    if (!qAttachment.isEmpty()) {
        FILE* file = fopen(attachmentPath.c_str(), "rb");
        if (file == nullptr) {
            ui->label_eroare2->setText("Fisierul atasament nu poate fi deschis sau nu exista.");
            return;
        } else {
            fclose(file);

         string filename = qAttachment.toStdString().substr(qAttachment.toStdString().find_last_of("/\\") + 1);
           string sendFileCommand = "SEND_FILE " + filename;

            write(sd, sendFileCommand.c_str(), sendFileCommand.length());

            // Asteapta confirmarea serverului
            char serverResponse[100];
            read(sd, serverResponse, sizeof(serverResponse));

            if (strncmp(serverResponse, "READY_TO_RECEIVE", 16) == 0) {
                file = fopen(attachmentPath.c_str(), "rb");
                char buffer[1024];
                int bytes_read;


                    // Citire si trimitere continut fisier
                    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    send(sd, buffer, bytes_read, 0);
                }

                const char* end_of_file_marker = "EOF";
                send(sd, end_of_file_marker, strlen(end_of_file_marker), 0);

                fclose(file);
            } else {
                ui->label_eroare2->setText("Serverul nu este pregatit pentru primirea fisierului.");
                return;
            }
        }
    }
    sleep(3);
    string message = "COMPOSE -to " + qTo.toStdString() + " -title " + qTitle.toStdString() + " -text " + qText.toStdString();

    if (!qAttachment.isEmpty()) {
       message += " -attachment " + qAttachment.toStdString();
    }

    // Trimite mesajul la server
    write(sd, message.c_str(), message.length());


    this->close();
}

