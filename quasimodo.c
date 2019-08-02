#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>

int sock = 0;
int valread;

enum
{
    URL_MAX = 2000,
    PORT = 3312
};

int
main (int argc, char *argv[])
{   
    char *allowpass = "SETAPMODE UNLOCKED ALL\r\n";
    char *closedoor = "SETAPMODE LOCKED ALL\r\n";
    sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket creation error.\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 configes from text to binary form
    if(inet_pton(AF_INET, "10.15.12.245", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid config/Address not supported.\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    FILE* lgn= fopen("login", "r");

    char *logininfo = calloc(1024, sizeof(*logininfo));
    fread(logininfo, 1024, sizeof(*logininfo), lgn);

    int loginlength = strlen(logininfo) + 1;

    char *login = calloc(12 + loginlength, sizeof(login));
    snprintf(login, 12 + loginlength, "LOGIN 1.8 %s\r\n", logininfo);

    send(sock, login, strlen(login), 0);
    for (int i = 0; i < 1024; i++) {
        buffer[i] = 0;
    }
    valread = read(sock, buffer, 1024);
    if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
        printf("Login Failed\n");
    }

    if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
        printf("Connection Failed");
    } else {
        // Closing the door to ensure normal operation
        send(sock, closedoor, strlen(closedoor), 0);
        for (int i = 0; i < 1024; i++) {
            buffer[i] = 0;
        }
        valread = read(sock, buffer, 1024);
        if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
            printf("\nSomething went wrong, door not closed.\n");
        } else {
            printf("\nDoor closed!\n");
        }

        // Opening the door, waiting 10 seconds to pass
        send(sock, allowpass, strlen(allowpass), 0);
        for (int i = 0; i < 1024; i++) {
            buffer[i] = 0;
        }
        valread = read(sock, buffer, 1024);
        if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
            printf("\nSomething went wrong, door not opened.\n");
        } else {
            printf("\nDoor opened!\n");
        }

        sleep(10);

        // Closing the door, terminating the application
        send(sock, closedoor, strlen(closedoor), 0);
        for (int i = 0; i < 1024; i++) {
            buffer[i] = 0;
        }
        valread = read(sock, buffer, 1024);
        if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
            printf("\nSomething went wrong, door not closed.\n");
        } else {
            printf("\nDoor closed!\n");
        }
    }

    while (wait(NULL) != -1);

    free(logininfo);

    return 0;
}
