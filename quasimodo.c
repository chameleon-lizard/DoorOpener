#include <sys/socket.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>

int sock = 0;
int valread;

enum
{
    RETURN_VALUE_SIZE = 1024,
    RETURN_LENGTH = 3,
    DISPLACEMENT = 5,
    CFG_MAX = 2000,
    PORT = 3312
};

int
main(void)
{
    char setap_unlocked[] = "SETAPMODE UNLOCKED ALL\r\n";
    char setap_normal[]   = "SETAPMODE NORMAL ALL\r\n";

    struct sockaddr_in serv_addr;
    char return_value[RETURN_VALUE_SIZE] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket not created. Terminating application\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Reading the opener IP address, login and password from the config
    FILE *config = fopen("config", "r");
    char *cfg = (char *)calloc(CFG_MAX, sizeof(*cfg));

    fread(cfg, CFG_MAX, sizeof(*cfg), config);
    char *oip = DISPLACEMENT + strstr(cfg, "OIP:");
    char *olp = DISPLACEMENT + strstr(cfg, "OLP:");
    *strstr(oip, "\n") = 0;
    *strstr(olp, "\n") = 0;

    if (inet_pton(AF_INET, oip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid config\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    char login[] = "LOGIN 1.8 %s\r\n";

    printf(login, olp);

    send(sock, login, strlen(login), 0);
    memset(return_value, 0, RETURN_VALUE_SIZE);

    valread = read(sock, return_value, RETURN_VALUE_SIZE);
    if (strlen(return_value) < 3 || !strncmp(return_value, "OK", RETURN_LENGTH)) {
        printf("Login failed\n");
    } else {
        // Closing door to ensure normal operation
        
        send(sock, setap_normal, strlen(setap_normal), 0);
        sleep(1); // ???

        memset(return_value, 0, RETURN_VALUE_SIZE);
        valread = read(sock, return_value, RETURN_VALUE_SIZE);
        if (strlen(return_value) < 3 || !strncmp(return_value, "OK", RETURN_LENGTH)) {
            printf("Door didn't close\n");
        } else {
            printf("Door closed\n");
        }

        // Opening door, waiting 5 seconds to pass
         
        send(sock, setap_unlocked, strlen(setap_unlocked), 0);
        sleep(1); // ???

        memset(return_value, 0, RETURN_VALUE_SIZE);
        valread = read(sock, return_value, RETURN_VALUE_SIZE);
        if (strlen(return_value) < 3 || !strncmp(return_value, "OK", RETURN_LENGTH)) {
            printf("Door didn't open\n");
        } else {
            printf("Door opened\n");
        }

        sleep(5);

        // Closing the door
         
        send(sock, setap_normal, strlen(setap_normal), 0);
        sleep(1); // ???

        memset(return_value, 0, RETURN_VALUE_SIZE);
        valread = read(sock, return_value, RETURN_VALUE_SIZE);
        if (strlen(return_value) < 3 || !strncmp(return_value, "OK", RETURN_LENGTH)) {
            printf("Door didn't close\n");
        } else {
            printf("Door closed\n");
        }
    }

    free(cfg);

    return 0;
}
