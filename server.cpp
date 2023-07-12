#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <cstdlib>
#include <sys/wait.h>
#include "write_read_mess.h"
using namespace std;

#define PORT 2728
extern int errno;

void write_response_read_command(int client, char response[], char command[]);
void get_commands(int client);
void login_cmd(int client, char user[30]);
void logout_cmd(int client, char user[30]);
void update_status(char status[30], char user[30]);
void register_cmd(int client, char user[30]);
void users_cmd();
void history_cmd(int client);
void delete_history_cmd(int client);
void open_cmd(int client);
void start_conversation(int client, char mess[500]);
bool verify_users_list(char user[30]);
bool file_exists(const char* fileName);


fstream file;
int nr = 0;
char response[500];
char command[500];
char current_user[30];
char current_receiver[30];
char conv_file[30]; // fisierul in care se va scrie istoricul conversatiei
char conv_file2[30];
bool logged = false;
bool conv_started = false;
bool unknown_command = false;
char reply_mess[100];


void get_commands(int client) {
    if (unknown_command) {
        strcat(response, "\nCommand not recognized! Try again!\n\n");
        unknown_command = false;
    }

    if (!logged) { // welcome page
        strcat(response, "Choose one of the following commands:\n");
        strcat(response, "-register      -> register to server \n");
        strcat(response, "-login         -> login to server\n");
        strcat(response, "-exit          -> leave server \n\n\n");
        write_response_read_command(client, response, command);

        if(strcmp(command, "-register") == 0) {
            strcat(response, "username: \n");
            write_response_read_command(client, response, command);
            register_cmd(client, command);
            get_commands(client);
        }
        else if(strcmp(command, "-login") == 0) {
            char user[500];
            strcpy(user, "username: \n");
            write_mess(client, user);
            read_mess(client, command);
            command[strlen(command)-1] = '\0';

            login_cmd(client, command);
            get_commands(client);
        }
        else if(strcmp(command, "-exit") == 0) {
            cout << "[server] S-a deconectat un client...\n";
        }
        else {
            unknown_command = true;
            get_commands(client);
        }
    }
    else {
        if (!conv_started) { // main menu page
            strcat(response, "\nChoose one of the following commands:\n");
            strcat(response, "-users            -> view all existent users \n");
            strcat(response, "-open             -> open conversation with a specific user\n");
            strcat(response, "-history          -> open conversation history with a specific user\n");
            strcat(response, "-delete history   -> delete conversation history with a specific user\n");
            strcat(response, "-logout           -> logout from the server\n");
            strcat(response, "-exit             -> leave server \n\n\n");
            write_response_read_command(client, response, command);

            if(strcmp(command, "-users") == 0) {
                users_cmd();
                get_commands(client);
                write_response_read_command(client, response, command);
            }
            else if(strcmp(command, "-open") == 0) {
                conv_started = true;
                open_cmd(client);
                get_commands(client);
            }
            else if(strcmp(command, "-history") == 0) {
                history_cmd(client);
                get_commands(client);
            }
            else if(strcmp(command, "-delete history") == 0) {
                delete_history_cmd(client);
                get_commands(client);
            }
            else if(strcmp(command, "-logout") == 0) {
                logout_cmd(client, current_user);
                strcat(response, "\n\nYou have logged out successfully!\n\n");
                get_commands(client);
            }
            else if(strcmp(command, "-exit") == 0) {
                logout_cmd(client, current_user);
                cout << "[server] S-a deconectat un client...\n";
            }
            else {
                unknown_command = true;
                get_commands(client);
            }
        }
        else { // conversation page
            if(nr == 0) {
                strcat(response, "\nWrite message or choose one of the following commands:\n");
                strcat(response, "-refresh           -> see new messages received\n");
                strcat(response, "-reply             -> reply to a specific message\n");
                strcat(response, "-back              -> go back to main menu\n\n\n");
                write_mess(client, response);
                response[0] = '\0';
            }
            nr++;
            if(strcmp(command, "-back") == 0) {
                conv_started = false;
                nr = 0;
                get_commands(client);
            }
            else {
                start_conversation(client, command);
                get_commands(client);
            }
        }
    }
}

bool file_exists(const char* fileName) {
    std::ifstream infile(fileName);
    return infile.good();
}

void start_conversation(int client, char mess[500]) {
    char curr_user_file[30], rec_file[30], message[500];

    strcpy(curr_user_file, current_user);
    strcat(curr_user_file, ".txt");

    if(file_exists(curr_user_file) == 1) {
        if (strcmp(command, "-refresh") == 0) {
            char line[100];
            file.open(curr_user_file);
            while (file.getline(line, 100)) {
                if (strstr(line, "-refresh") == nullptr && strstr(line, "-back") == nullptr && strstr(line, "-reply") == nullptr) {
                    strcat(line, "\n");
                    write_mess(client, line);
                }
            }
            file.close();
            ofstream fout(curr_user_file);
            fout << "";
            fout.close();
        }
    }

    read_mess(client, command);
    command[strlen(command)-1] = '\0';

    if(strcmp(command, "-reply") == 0) {
        strcat(response, "message:\n");
        write_response_read_command(client, response, command);
        strcpy(reply_mess, command);

        if (file_exists(conv_file) == 1) {
            char line[100];
            int found = false;
            file.open(conv_file);
            while(found == false) {
                while (file.getline(line, 100)) {
                    if (strstr(line, reply_mess) != nullptr) {
                        if (strstr(line, "-refresh") == nullptr && strstr(line, "-back") == nullptr &&
                            strstr(line, "-reply") == nullptr) {
                            found = true;

                            strcpy(message, current_user);
                            strcat(message, " replied to \"");
                            strcat(message, reply_mess);
                            strcat(message, "\": ");

                            read_mess(client, command);
                            command[strlen(command) - 1] = '\0';

                            strcat(message, command);
                            strcat(message, "\n");

                        }
                    }
                }
                if(found == false) {
                    strcpy(message, "message doesn't exist!\n\n");
                    strcat(message, "message:\n");
                    read_mess(client, command);
                    command[strlen(command) - 1] = '\0';
                }
            }
            file.close();
        }
    }
    else {
        strcpy(message, current_user);
        strcat(message, ": ");
        strcat(message, command);
        strcat(message, "\n");
    }

    ofstream out(conv_file, std::ios::app);
    if (strstr(message, "-refresh") == nullptr && strstr(message, "-back") == nullptr && strstr(message, "-reply") == nullptr) {
        out << message;
    }
    out.close();

    ofstream out2(conv_file2, std::ios::app);
    if (strstr(message, "-refresh") == nullptr && strstr(message, "-back") == nullptr && strstr(message, "-reply") == nullptr) {
        out2 << message;
    }
    out2.close();

    strcpy(rec_file, current_receiver);
    strcat(rec_file, ".txt");

    ofstream g(rec_file, std::ios::app);
    g << message;
    g.close();
}

bool verify_users_list(char user[30]) {
    char my_users[50];
    file.open("users.txt");
    user[strlen(user)] = '\0';
    while(file >> my_users) {
        char *ptr;
        ptr = strtok(my_users, "-");
        if (ptr != nullptr) {
            if(strcmp(user, ptr) == 0) {
                file.close();
                return true;
            }
        }
    }
    file.close();
    return false;
}

void update_status(char status[30], char user[30]) {
    char user_status[50], user_status_updated[50]; // facem update la status dupa logare
    file.open("users.txt");
    ofstream fout("temp.txt");
    while(file >> user_status) {
        char aux[50];
        strcpy(aux, user_status);
        char *ptr;
        ptr = strtok(aux, "-");
        if (ptr != nullptr) {
            if(strcmp(ptr, user) == 0){
                strcpy(user_status_updated, ptr);
                user_status_updated[strlen(user_status_updated)] = '\0';
                strcat(user_status_updated, status);
                fout <<user_status_updated << '\n';
            }
            else {
                fout << user_status << '\n';
            }
        }
    }
    remove("users.txt");
    rename("temp.txt", "users.txt");
    file.close();
    fout.close();
}

void register_cmd(int client, char user[30]) {
    char user_info[50], username[30], password[30];
    bool user_exists = false;

    file.open("database.txt");
    while(file >> user_info) {
        char *ptr;
        ptr = strtok(user_info, ":");
        int cnt = 0;
        while (ptr != nullptr) {
            if(cnt == 0) {
                strcpy(username, ptr);
            }
            else {
                strcpy(password, ptr);
            }
            cnt++;
            ptr = strtok (nullptr, ":");
        }

        if(strcmp(user, username) == 0) { // userul exista deja in baza de date
            user_exists = true;
        }
    }

    file.close();
    if(user_exists) {
        strcat(response, "Username already exists! Choose another one!\n\n");
        strcat(response, "username: \n");
        write_response_read_command(client, response, command);
        register_cmd(client, command);
    }
    else {
        char pass_verify[30];
        file.open("database.txt", std::ios::app);
        file << user << ':';
        char username[30];
        strcpy(username, user);
        while(strcmp(pass_verify, command) != 0) {
            strcat(response, "password: \n");
            write_response_read_command(client, response, command);
            strcpy(pass_verify, command);
            strcat(response, "confirm password: \n");
            write_response_read_command(client, response, command);
        }
        file << pass_verify << '\n';
        file.close();
        strcat(response, "\n\nYou have registered successfully!\n\n");

        logged = true;
        strcpy(current_user, username);

        file.open("users.txt", std::ios::app);
        file << username << "-online" << '\n';
        file.close();
    }
}

void login_cmd(int client, char user[30]) {
    char user_info[50], username[30], password[30];
    bool user_exists = false;

    while(!user_exists) {
        file.open("database.txt");
        while(file >> user_info) {
            char *ptr;
            ptr = strtok(user_info, ":");
            int cnt = 0;
            while (ptr != nullptr) {
                if(cnt == 0) {
                    strcpy(username, ptr);
                }
                else {
                    strcpy(password, ptr);
                }
                cnt++;
                ptr = strtok (nullptr, ":");
            }

            if(strcmp(user, username) == 0) { // userul exista deja in baza de date
                user_exists = true;
                break;
            }
        }
        file.close();
        if(user_exists) {
            break;
        }
        strcat(response, "\nUsername doesn't exist!\n");
        strcat(response, "username: \n");
        write_response_read_command(client, response, command);
    }
    char pass[500];
    strcpy(pass, "password: \n");
    write_mess(client, pass);
    read_mess(client, command);
    command[strlen(command)-1] = '\0';

    while(strcmp(command, password) != 0) {
        strcat(response, "\nWrong password! Try again! \n");
        strcat(response, "password: \n");
        write_response_read_command(client, response, command);
    }

    strcat(response, "\n\nYou have logged in successfully!\n\n");

    logged = true;
    strcpy(current_user, username);
    char status[30];
    strcpy(status, "-online");
    update_status(status, username);

    strcat(response, "New messages:\n");
    write_mess(client, response);
    response[0] = '\0';

    char curr_user_file[30];

    strcpy(curr_user_file, current_user);
    strcat(curr_user_file, ".txt");

    if(file_exists(curr_user_file) == 1) {
        char line[100];
        file.open(curr_user_file);
        while (file.getline(line, 100)) {
            if (strstr(line, "-refresh") == nullptr && strstr(line, "-back") == nullptr &&
                strstr(line, "-reply") == nullptr) {
                strcat(line, "\n");
                write_mess(client, line);
            }
        }
        file.close();
        ofstream fout(curr_user_file);
        fout << "";
        fout.close();
    }
}

void users_cmd() {
    strcat(response, "\nList of users:\n");
    char user_status[50];
    file.open("users.txt");
    while(file >> user_status) {
        strcat(response, user_status);
        strcat(response, "\n");
    }
    file.close();
    strcat(response, "\n");
}

void open_cmd(int client) {
    strcat(response, "Username: \n");
    write_response_read_command(client, response, command);
    while(!verify_users_list(command)) {
        strcat(response, "\nChoose an existent user! \n\n");
        strcat(response, "Username: \n");
        write_response_read_command(client, response, command);
    }
    strcpy(current_receiver, command);

    strcpy(conv_file, current_user);
    strcat(conv_file, "_");
    strcat(conv_file, current_receiver);
    strcat(conv_file, ".txt");

    strcpy(conv_file2, current_receiver);
    strcat(conv_file2, "_");
    strcat(conv_file2, current_user);
    strcat(conv_file2, ".txt");

    char mess[30];
    strcpy(mess, "\nConversation has started!\n");
    write_mess(client, mess);
}

void history_cmd(int client) {
    strcpy(response, "\nUsername: \n");
    write_response_read_command(client, response, command);
    while(!verify_users_list(command)) {
        strcpy(response, "\nChoose an existent user! \n\n");
        strcat(response, "Username: \n");
        write_response_read_command(client, response, command);
    }
    strcpy(current_receiver, command);

    strcpy(conv_file, current_user);
    strcat(conv_file, "_");
    strcat(conv_file, current_receiver);
    strcat(conv_file, ".txt");

    char mess[30];
    strcpy(mess, "\nHistory:\n\n");
    write_mess(client, mess);

    if(file_exists(conv_file) == 1) {
        if (strstr(conv_file, command) != nullptr) { // fisierul are numele celui cu care vreau sa incep o conversatie
            char line[100];
            file.open(conv_file);
            while (file.getline(line, 100)) {
                strcat(line, "\n");
                write_mess(client, line);
            }
            file.close();
        }
    }
}

void delete_history_cmd(int client) {
    strcat(response, "Username: \n");
    write_response_read_command(client, response, command);
    while(!verify_users_list(command)) {
        strcat(response, "\nChoose an existent user! \n\n");
        strcat(response, "Username: \n");
        write_response_read_command(client, response, command);
    }

    strcpy(current_receiver, command);

    strcpy(conv_file, current_user);
    strcat(conv_file, "_");
    strcat(conv_file, current_receiver);
    strcat(conv_file, ".txt");

    if(file_exists(conv_file) == 1) {
        if (strstr(conv_file, command) != nullptr) {
            ofstream fout(conv_file);
            fout << "";
            fout.close();
        }
    }
}

void logout_cmd(int client, char user[30]) {
    logged = false;
    conv_started = false;
    char status[30];
    strcpy(status, "-offline");
    update_status(status, user);
}

void write_response_read_command(int client, char response[], char command[]) {
    int length_response = strlen(response);
    int length_command;

    if(write(client, &length_response, sizeof(length_response)) <= 0) { // scriu clientului ce dimensiunea mesajului ce va urma sa l citeasca
        perror("[server] Eroare la write() catre client.\n");
    }

    if (write(client, response, length_response) <= 0) { // scrie catre client mesajul generat de server
        perror("[server] Eroare la write() catre client.\n");
    }

    if (read(client, &length_command, sizeof(length_command)) <= 0) { // citeste de la client dimensiunea mesajului ce umeaza citit
        perror(" Eroare la Read:");
        printf("[server] S-a deconectat un client...");
        close(client);
    }

    bzero(command, 500);
    if (read(client, command, length_command) <= 0) { // citeste de la client mesajul
        perror("[server] Eroare la read() de la client.\n");
        close(client);
    }

    command[strlen(command) - 1] = '\0';
    bzero(response, 500);
}

int main() {
    struct sockaddr_in server{};
    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;                  /* stabilirea familiei de socket-uri */
    server.sin_addr.s_addr = htonl(INADDR_ANY);  /* acceptam orice adresa */
    server.sin_port = htons(PORT);              /* utilizam un port utilizator */

    struct sockaddr_in client_str{};
    bzero(&client_str, sizeof(client_str));

    /* crearea unui socket */
    int sd;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[SERVER]: Eroare la socket().\n");
        return errno;
    }

    /* setarea optiunii SO_REUSEADDR pentru socket - permite atasarea la un port/adresa deja in uz*/
    int on = 1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    /* atasam socketul la adresa */
    if (bind(sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        perror("[SERVER]: Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 5) == -1) {
        perror("[SERVER]: Eroare la listen().\n");
        return errno;
    }

    printf("[SERVER]: Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    // servirea clientilor in mod concurent
    while (1) {
        int client;
        socklen_t len = sizeof(client_str);

        /* acceptam conexiunea cu noul client */
        client = accept(sd, (struct sockaddr *) &client_str, &len);

        /* eroare la acceptarea conexiunii de la un client */
        if (client < 0) {
            perror ("[SERVER]: Eroare la accept().\n");
            continue;
        }

        int pid;
        if ((pid = fork()) == -1) { // nu s-a putut crea un proces copil
            close(client);
            continue;
        } else if (pid > 0) {
            while (waitpid(-1, NULL, WNOHANG)); //asteapta ca procesul copil sa se termine
            continue;
        } else if (pid == 0) {
            close(sd);

            bzero(command, 500);

            printf("[SERVER]: S-a conectat clientul cu descriptorul %d\n", client);
            strcat(response, "Welcome to Offline Messenger!\n\n");

            get_commands(client);

            close(client);
            exit(0);
        }
    }
}