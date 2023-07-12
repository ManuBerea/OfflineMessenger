#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include "write_read_mess.h"
using namespace std;

extern int errno;

int main (int argc, char *argv[]) {

    if (argc != 3) {
        printf("[CLIENT]: Sintaxa: %s <ip> <port>\n", argv[0]);
        return -1;
    }

    int port = atoi (argv[2]);
    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    struct sockaddr_in server{};
    server.sin_family = AF_INET;                 /* familia socket-ului */
    server.sin_addr.s_addr = inet_addr(argv[1]); /* adresa IP a serverului */
    server.sin_port = htons(port);      /* portul de conectare */

    int sd;
    if((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[CLIENT]: Eroare la socket().\n");
        return errno;
    }

    if(connect(sd, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1){
        perror("[CLIENT]: Eroare la connect().\n");
        return errno;
    }

    int fiu = fork();
    if(fiu == 0) {
        char mess[500];
        while (1) {
            read_mess(sd, mess);
            cout << mess;
        }
    }
    else {
        char message[500];
        while(1) {
            fflush(stdout);
            bzero(message, 500);
            read(0, message, 500);

            write_mess(sd, message);

            message[strlen(message)-1] = '\0';
            if (strcmp(message, "-exit") == 0) {
                cout << "\nGoodbye!!\n\n";
                close(sd);
                kill(fiu, SIGQUIT);
                exit(0);
            }
        }
    }
}