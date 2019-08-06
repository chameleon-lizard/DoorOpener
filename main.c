#include <sys/socket.h>
#include <arpa/inet.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>

int sock = 0;
int valread;
char *oip;
char *olp;
char *sip;

enum
{
    RETURN_VALUE_SIZE = 1024,
    RETURN_LENGTH = 3,
    DISPLACEMENT = 5,
    CFG_MAX = 2000,
    PORT = 3312
};

int
opendoor(GtkWidget *widget, gpointer data)
{
    char allowpass[] = "ALLOWPASS 1 ANONYMOUS IN\r\n";

    struct sockaddr_in serv_addr;
    char return_value[RETURN_VALUE_SIZE] = {0};
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

    valread = read(sock, return_value, RETURN_VALUE_SIZE);
    if (strlen(return_value) < 3 || !strncmp(return_value, "OK", RETURN_LENGTH)) {
        printf("Login failed\n");
    } else {
        // Allowing pass
         
        send(sock, allowpass, strlen(allowpass), 0);
        sleep(1); // ???

        memset(return_value, 0, RETURN_VALUE_SIZE);
        valread = read(sock, return_value, RETURN_VALUE_SIZE);
        if (strlen(return_value) < 3 || !strncmp(return_value, "OK", RETURN_LENGTH)) {
            printf("Door didn't open\n");
        } else {
            printf("Door opened\n");
        }
    }


    free(login);

    return 0;
}

void destroy(GtkWidget *widget, gpointer data )
{
    gtk_main_quit();
}

int
main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *button;
    gtk_init(&argc, &argv);

    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    int pid = 0;

     // Reading the opener IP address, login and password from the config
    FILE *config = fopen("config", "r");
    char *cfg = (char *)calloc(CFG_MAX, sizeof(*cfg));

    fread(cfg, CFG_MAX, sizeof(*cfg), config);

    fclose(config);

    sip = DISPLACEMENT + strstr(cfg, "SIP:");
    oip = DISPLACEMENT + strstr(cfg, "OIP:");
    olp = DISPLACEMENT + strstr(cfg, "OLP:");
    *strstr(sip, "\n") = 0;
    *strstr(oip, "\n") = 0;
    *strstr(olp, "\n") = 0;

    if (!(pid = fork())) {
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
        gtk_container_set_border_width(GTK_CONTAINER(window), 1);
        button = gtk_button_new_with_label("Open door");
        g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(opendoor), "button");
        gtk_container_add(GTK_CONTAINER(window), button);
        gtk_widget_show_all(window);
        gtk_main();
    } else {
        gst_init(&argc, &argv);
        pipeline = gst_parse_launch(sip, NULL);
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        bus = gst_element_get_bus(pipeline);
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    }

    while (wait(NULL) != -1);

    // Free resources
    if (msg != NULL) {
        gst_message_unref(msg);
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    free(cfg);

    return 0;
;}
