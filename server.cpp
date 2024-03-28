#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <sqlite3.h>

using namespace std;
#define PORT 110
#define MSG_SIZE 1024

//FUNCTII SISTEM LOGARE
int is_user_logged_in(const char *username) {
    FILE *file = fopen("active_users.txt", "r");
    if (!file) {
        return 0;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; //ELIMINARE NEWLINE
        if (strcmp(line, username) == 0) {
            fclose(file);
            return 1; //UTILIZATOR LOGAT
        }
    }

    fclose(file);
    return 0; 
}

void add_user_to_active(const char *username) {
    FILE *file = fopen("active_users.txt", "a");
    if (file) {
        fprintf(file, "%s\n", username);
        fclose(file);
    }
}

void remove_user_from_active(const char *username) {
    FILE *file = fopen("active_users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    char line[100];

    if (!file || !temp) {
        if (file) fclose(file);
        if (temp) fclose(temp);
        return;
    }
        while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, username) != 0) {
            fprintf(temp, "%s\n", line);
        }
    }

    fclose(file);
    fclose(temp);
    remove("active_users.txt");
    rename("temp.txt", "active_users.txt");
}
    
int verify_user(const char *username, const char *password) {
    FILE *file = fopen("users.txt", "r");
    if (file == NULL) {
        perror("Eroare la deschiderea fisierului de utilizatori");
        return 0;
    }

    char line[256];
    char user[100], pass[100];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%20[^:]:%20s", user, pass);
        if (strcmp(username, user) == 0 && strcmp(password, pass) == 0) {
            fclose(file);
            return 1; // PAROLA+UTILIZATOR VALIDE
        }
    }

    fclose(file);
    return 0; // NEVALIDE
}

//FUNCTII SISTEM MAILING

void send_email(const string& to, const string& from, const string& title, const string& text, const string& attachment) {
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    // Deschide baza de date
    rc = sqlite3_open("email_database.db", &db);
    if (rc) {
        std::cerr << "Eroare la deschiderea bazei de date: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // Creeaza un tabel pentru e-mailuri daca nu exista
const char *sqlCreateTable = "CREATE TABLE IF NOT EXISTS emails ("
                             "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "DESTINATAR TEXT, "
                             "EXPEDITOR TEXT, "
                             "TITLE TEXT, "
                             "TEXT TEXT, "
                             "ATTACHMENT TEXT);";

    rc = sqlite3_exec(db, sqlCreateTable, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Eroare SQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    // Comanda SQL pentru inserarea unui nou e-mail
    const char *sqlInsert = "INSERT INTO emails (DESTINATAR, EXPEDITOR, TITLE, TEXT, ATTACHMENT) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "Eroare la pregatirea insertului: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // Legături pentru parametri
    sqlite3_bind_text(stmt, 1, to.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, from.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, attachment.c_str(), -1, SQLITE_STATIC);

    // Executa insertul
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "Eroare la inserarea in baza de date: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "E-mail inserat cu succes." << endl;
    }

    // Elibereaza resursele
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}



int main()
{
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    //-------------------------INITIALIZARE SQL------------------------
    sqlite3 *db;
    if (sqlite3_open("email_database.db", &db)) {
        cerr << "Eroare la deschiderea bazei de date: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    // Creeaza un tabel pentru e-mailuri daca nu exista
  const char *sqlCreateTable = "CREATE TABLE IF NOT EXISTS emails ("
                             "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "DESTINATAR TEXT, "
                             "EXPEDITOR TEXT, "
                             "TITLE TEXT, "
                             "TEXT TEXT, "
                             "ATTACHMENT TEXT);";

    char *errMsg = 0;
    if (sqlite3_exec(db, sqlCreateTable, 0, 0, &errMsg) != SQLITE_OK) {
        cerr << "Eroare SQL: " << errMsg << endl;
        sqlite3_free(errMsg);
    }
    sqlite3_close(db);
        //-------------------------FINAL------------------------

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    if (listen(sd, 1) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    while (1)
    {
        int client;
        socklen_t length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        client = accept(sd, (struct sockaddr *)&from, &length);

        if (client < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        int pid;
        if ((pid = fork()) == -1)
        {
            close(client);
            continue;
        }
        else if (pid > 0)
        {
            close(client);
            while (waitpid(-1, NULL, WNOHANG))
                ;
            continue;
        }
     else if (pid == 0) {
    close(sd);
    char username[100] = {0};
    char password[100] = {0};
 char msg[MSG_SIZE];
    char msgrasp[20000];
    while(1) {
        bzero(msg, MSG_SIZE);
        bzero(msgrasp,20000);
        printf("[server]Asteptam mesajul...\n");
        fflush(stdout);

        if (read(client, msg, MSG_SIZE) <= 0) {
            perror("[server]Eroare la read() de la client.\n");
                if(is_user_logged_in(username))
                remove_user_from_active(username);
            break;
        }

        printf("[server]Mesajul a fost receptionat...%s\n", msg);

if (strncmp(msg, "LOGIN ", 6) == 0) {
    // Extragere username și password
    sscanf(msg+ 6, "%20s %20s", username, password); // Citim două string-uri separate de spațiu

    printf("USERNAME: %s\n", username);
    printf("PASSWORD: %s\n", password);

    if (is_user_logged_in(username)) {
        strcpy(msgrasp, "-ERR Utilizator deja logat");
    } else {
        if (verify_user(username, password)) {
            printf("OK\n");

            strcpy(msgrasp, "+OK Autentificare reusita");
            add_user_to_active(username);
        } else {
            strcpy(msgrasp, "-ERR Autentificare esuata, username sau parola gresita!");
        }
    }

    write(client, msgrasp, strlen(msgrasp));

}


else if (strcmp(msg, "LOGOUT") == 0) {
    remove_user_from_active(username);
    bzero(username, sizeof(username)); //Resetare nume utilizator
    bzero(password, sizeof(password)); //Resetare parola

}

else if (strncmp(msg, "COMPOSE", 7) == 0) {
    if (is_user_logged_in(username)) {
        string msg_str(msg);
        size_t to_pos = msg_str.find("-to ") + 4;
        size_t title_pos = msg_str.find("-title ") + 7;
        size_t text_pos = msg_str.find("-text ") + 6;
        size_t attachment_pos = msg_str.find("-attachment ");

        string to = msg_str.substr(to_pos, msg_str.find(" -title ", to_pos) - to_pos);
        string title = msg_str.substr(title_pos, msg_str.find(" -text ", title_pos) - title_pos);
        string text = msg_str.substr(text_pos, (attachment_pos != string::npos) ? msg_str.find(" -attachment ", text_pos) - text_pos : msg_str.find("\n", text_pos) - text_pos);
        string attachment = (attachment_pos != string::npos) ? msg_str.substr(attachment_pos + 12, msg_str.find("\n", attachment_pos + 12) - (attachment_pos + 12)) : "";


        if (attachment_pos == string::npos) {
            attachment.clear();
        }

        if (!attachment.empty()) {
            size_t last_slash_pos = attachment.find_last_of("/\\");
            string attachment_name = last_slash_pos != string::npos ? attachment.substr(last_slash_pos + 1) : attachment;
            send_email(to, username, title, text, attachment_name);
            strcpy(msgrasp, attachment.c_str());
        } else {
            // Trimitere mail atasament
            send_email(to, username, title, text, "");
            strcpy(msgrasp, "+OK E-mail trimis.\n");
        }
    } else {
        strcpy(msgrasp, "-ERR Logheaza-te mai intai.\n");
    }
    write(client, msgrasp, strlen(msgrasp));
}




else if (strncmp(msg, "LIST_SENT", 9) == 0) {
    if (is_user_logged_in(username)) {
        int page;
        if (sscanf(msg, "LIST_SENT %d", &page) != 1 || page < 1) {
            strcpy(msgrasp, "-ERR Pagina invalida.\n");
        } else {
            sqlite3 *db;
            sqlite3_stmt *stmt;

            if (sqlite3_open("email_database.db", &db)) {
                strcpy(msgrasp, "-ERR Eroare la deschiderea bazei de date\n");
            } else {
                std::string sql = "SELECT ID, TITLE, EXPEDITOR, DESTINATAR FROM emails WHERE EXPEDITOR = ? ;";
                sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
           sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
                std::string response;
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    response += "ID: " + std::to_string(sqlite3_column_int(stmt, 0)) + "\n";
response += "Title: " + std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))) + "\n";
response += "To: " + std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))) + "\n\n";
}
cout<<response.length()<<endl<<response;
            if (response.empty()) {
                strcpy(msgrasp, "Nu exista e-mail-uri trimise pe aceasta pagina.\n");
            } else {
                strcpy(msgrasp, response.c_str());
            }

            sqlite3_finalize(stmt);
            sqlite3_close(db);
        }
    }
} else {
    strcpy(msgrasp, "-ERR Logheaza-te mai intai.\n");
}
write(client, msgrasp, strlen(msgrasp));

}

else if (strncmp(msg, "LIST_RECV", 9) == 0) {
    if (is_user_logged_in(username)) {
        int page;
        if (sscanf(msg, "LIST_RECV %d", &page) != 1 || page < 1) {
            strcpy(msgrasp, "-ERR Pagina invalida.\n");
        } else {
            sqlite3 *db;
            sqlite3_stmt *stmt;

            if (sqlite3_open("email_database.db", &db)) {
                strcpy(msgrasp, "-ERR Eroare la deschiderea bazei de date\n");
            } else {
                std::string sql = "SELECT ID, TITLE, EXPEDITOR FROM emails WHERE DESTINATAR = ? ;";
                sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
                sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

                std::string response;
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    response += "ID: " + to_string(sqlite3_column_int(stmt, 0)) + "\n";
                    response += "Title: " + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))) + "\n";
                    response += "From: " + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))) + "\n\n";
                }
cout<<response.length()<<endl<<response;
                if (response.empty()) {
                    strcpy(msgrasp, "Nu exista e-mail-uri primite pe aceasta pagina.\n");
                } else {
                    strcpy(msgrasp, response.c_str());
                }

                sqlite3_finalize(stmt);
                sqlite3_close(db);
            }
        }
    } else {
        strcpy(msgrasp, "-ERR Logheaza-te mai intai.\n");
    }
    write(client, msgrasp,strlen(msgrasp));
}

else if (strncmp(msg, "OPEN", 4) == 0) {
    if (is_user_logged_in(username)) {
        int email_id;
        if (sscanf(msg, "OPEN %d", &email_id) == 1) {
            sqlite3 *db;
            sqlite3_stmt *stmt;

            if (sqlite3_open("email_database.db", &db)) {
                strcpy(msgrasp, "-ERR Eroare la deschiderea bazei de date\n");
            } else {
                std::string sql = "SELECT ID, TITLE, EXPEDITOR, DESTINATAR, TEXT, ATTACHMENT FROM emails WHERE ID = ? AND (EXPEDITOR = ? OR DESTINATAR = ?);";
                sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
                sqlite3_bind_int(stmt, 1, email_id);
                sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, username, -1, SQLITE_STATIC);

                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    std::string response = "ID: " + to_string(sqlite3_column_int(stmt, 0)) + "\n";
                    response += "Title: " + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))) + "\n";
                    response += "From: " + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))) + "\n";
                    response += "To: " + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))) + "\n";
                    response += "Text: " + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))) + "\n";
                    response += "Attachment: " + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))) + "\n";
                    strcpy(msgrasp, response.c_str());

                } else {
                    strcpy(msgrasp, "E-mail-ul specificat nu exista sau nu aveti acces la el.\n");
                }

                sqlite3_finalize(stmt);
                sqlite3_close(db);
            }
        } else {
            strcpy(msgrasp, "-ERR Format comanda incorect.\n");
        }
    } else {
        strcpy(msgrasp, "-ERR Logheaza-te mai intai.\n");
    }

    write(client, msgrasp, strlen(msgrasp));
}


 
 else if (strncmp(msg, "DOWNLOAD", 8) == 0) {
    char file_name[100];
    sscanf(msg, "DOWNLOAD %s", file_name); // Extract file name from command

    char file_path[200];
    sprintf(file_path, "/home/kali/Desktop/attachments/%s", file_name); // Path where the file is stored

    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("File not found or unable to open");
        strcpy(msgrasp, "ERROR: File not found or unable to open.\n");
        write(client, msgrasp, strlen(msgrasp));
        continue;
    }


    // Sending file content
    char buffer[1024];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client, buffer, bytes_read, 0);
        printf("bytes:%d\n",bytes_read);
    }

    // Send end-of-file marker
    const char* end_of_file_marker = "EOF";
    send(client, end_of_file_marker, strlen(end_of_file_marker), 0);

    fclose(file);
    printf("File has been sent.\n");
}

   

else if (strncmp(msg, "SEND_FILE", 9) == 0) {


    char file_name[100];
    sscanf(msg, "SEND_FILE %s", file_name); // Extragere nume fisier din comanda

    char file_path[200];
    sprintf(file_path, "/home/kali/Desktop/attachments/%s", file_name); // Calea unde va fi salvat fisierul
 FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        perror("Nu s-a putut deschide fisierul pentru scriere");

         strcpy(msgrasp, "Nu s-a putut deschide fisierul pentru scriere.\n");
            write(client, msgrasp, strlen(msgrasp));
                    break;
    }


    char response[] = "READY_TO_RECEIVE";
    write(client, response, sizeof(response));

   
    // Primire și scriere fisier
    char buffer[1024];
    int bytes_read;
bool end_of_file = false;
while (!end_of_file && (bytes_read = read(client, buffer, sizeof(buffer))) > 0) {
    printf("Primiti:%d \n",bytes_read);
  if (bytes_read >= 3 && strncmp(buffer + bytes_read - 3, "EOF", 3) == 0) {
        // Scrie totul in fisier in afara de EOF
        fwrite(buffer, 1, bytes_read - 3, file);
                printf("iesire din while");
        break;

    } else {
        fwrite(buffer, 1, bytes_read, file);
    }
}

fclose(file);
        strcpy(msgrasp, "+OK Fisierul a fost primit si salvat.\n");
            write(client, msgrasp, strlen(msgrasp));
     
}



else if (strcmp(msg, "QUIT") == 0) {
    if (strlen(username) > 0) {
        remove_user_from_active(username);
    }
    printf("[server]Comanda QUIT primita. Deconectare client.\n");
    break;
}
else {
            strcpy(msgrasp, "-ERR Comandă necunoscută\n");
            write(client, msgrasp, strlen(msgrasp));
        }
    }

    close(client);
    exit(0);
}
    }
}
