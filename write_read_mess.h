#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <cstring>

void write_mess(int d, char mess[500]) {
    int length_mess = strlen(mess);
    if(write(d, &length_mess, sizeof(int)) <= 0){
        perror("Eroare la write");
    }

    if (write(d, mess, length_mess) <= 0) {
        perror("Eroare la write");
    }
}

void read_mess(int d, char mess[500]) {
    int length_mess;
    bzero(mess, 500);

    if (read(d, &length_mess, sizeof(int)) < 0) {
        perror("Eroare la read");
    }
    if (read(d, mess, length_mess) < 0) {
        perror("Eroare la read");
    }
}
