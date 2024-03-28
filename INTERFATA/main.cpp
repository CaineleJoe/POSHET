#include "mainwindow.h"
#include "global.h"
#include <QApplication>
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

int sd;
int main(int argc, char *argv[])
{
    struct sockaddr_in server;
    int port;

    port =110;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return -1;
    }
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
     a.exec();
    close(sd);
     return 0;
}
