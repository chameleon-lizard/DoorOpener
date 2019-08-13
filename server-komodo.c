#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

enum
{
    RETURN_VALUE_SIZE = 1024,
    RETURN_LENGTH = 3,
    DISPLACEMENT = 5,
    CFG_MAX = 2000,
    PORT = 3312
};


int
main(int argc, char *argv[])
{
    if (argc == 1) {
        return -1;
    } else {
        int sock = 0;
        int valread;
    
        // Reading the opener IP address, login and password from the config
        FILE *config = fopen("config", "r");
        char *cfg = (char *)calloc(CFG_MAX, sizeof(*cfg));
    
        char *oip;
        char *olp;
        
        fread(cfg, CFG_MAX, sizeof(*cfg), config);
        fclose(config);
    
        oip = DISPLACEMENT + strstr(cfg, "OIP:");
        olp = DISPLACEMENT + strstr(cfg, "OLP:");
        *strstr(oip, "\n") = 0;
        *strstr(olp, "\n") = 0;
    
        // Logging into sphinx
    
        char return_value[RETURN_VALUE_SIZE] = {0};                    
        struct sockaddr_in serv_addr;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket not created. Terminating application\n");
            return -1;
        }
    
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
    
        if (inet_pton(AF_INET, oip, &serv_addr.sin_addr) <= 0) {
            printf("Invalid config\n");
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("Connection Failed\n");
            return -1;
        }

        char *login = (char *)calloc(strlen("LOGIN 1.8 \r\n") + strlen(olp) + 1, sizeof(*login));
    
        snprintf(login, strlen("LOGIN 1.8 \r\n") + strlen(olp), "LOGIN 1.8 %s\n\r", olp);
    
        send(sock, login, strlen(login), 0);
        memset(return_value, 0, RETURN_VALUE_SIZE);
    
        free(login);
    
        valread = read(sock, return_value, RETURN_VALUE_SIZE);
        if (strlen(return_value) < 3 || !strncmp(return_value, "OK", RETURN_LENGTH)) {
            printf("Login failed\n");
        } else {
            // Allowing pass
            char *cmd= (char *)calloc(strlen("ALLOWPASS 1 %s IN\r\n") + 2 + 1, sizeof(*cmd));
            snprintf(cmd, strlen("ALLOWPASS 1 %s IN\r\n") + 2, "ALLOWPASS 1 %s IN\r\n", argv[1]);
            send(sock, cmd, sizeof(*cmd) * strlen(cmd), 0);
            sleep(1);
        
            memset(return_value, 0, RETURN_VALUE_SIZE);
            valread = read(sock, return_value, RETURN_VALUE_SIZE);
            if (strlen(return_value) < 3 || !strncmp(return_value, "OK", RETURN_LENGTH)) {
                printf("Door didn't open\n");
            } else {
                printf("Door opened\n");
            }
        }
    
        close(sock);
        free(cfg);
    }
}
