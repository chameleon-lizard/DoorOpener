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

    send(sock, hello, strlen(hello), 0);
    valread = recv(sock, buffer, 1024, 0);
    if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
        printf("Connection Failed");
    } else {
        char *LOGIN = "LOGIN 1.8 %s %s\r\n";

        if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
            printf("Login Failed\n");
        }

        char *allowpass = "ALLOWPASS 1 17 IN\r\n";
        if (!isOpened) {
            send(sock, allowpass, strlen(allowpass), 0);
            char buffer[1024] = { 0 };
            valread = read(sock, buffer, 1024);
            if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
                printf("\nSomething went wrong, door not opened.\n");
            } else {
                printf("\nDoor opened!\n");
            }
    }

    while (wait(NULL) != -1);

    // Free resources
    if (msg != NULL) {
        gst_message_unref(msg);
    }
    
    return 0;
}
