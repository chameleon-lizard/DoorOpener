#include <sys/socket.h>
#include <arpa/inet.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
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
    PORT = 0
};

static int isOpened = 0;

void greet(GtkWidget *widget, gpointer data)
{
    char allowpass[17] = "ALLOWPASS 1 1 IN";
    if (!isOpened) {
        send(sock, allowpass, strlen(allowpass), 0);
        char buffer[1024] = { 0 };
        valread = read(sock, buffer, 1024);
        if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
            g_print("\nSomething went wrong, door not opened.\n");
        } else {
            g_print("\nDoor opened!\n");
        }
    }
}

void destroy(GtkWidget *widget, gpointer data )
{
    gtk_main_quit();
}

int
main (int argc, char *argv[])
{   
    GtkWidget *window;
    GtkWidget *button;
    gtk_init(&argc, &argv);

    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;
    char *config = calloc(2000, sizeof(*config));
    
    FILE *cfg = 0;
 
    int pid = 0;

    sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "LOGIN";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        g_print("\nSocket creation error.\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 configes from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        g_print("\nInvalid config/Address not supported.\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        g_print("\nConnection Failed \n");
        return -1;
    }

    send(sock, hello, strlen(hello), 0);
    valread = read(sock, buffer, 1024);
    if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
        g_print("Connection Failed");
    } else {
        // Logging in. 1.8 is the version of used protocol. TODO: get USER and PASSWD from config.
        
        // Reading the stream link from the config.
        if (!(cfg = fopen("config", "r"))) {
            g_print("\nConfig file not found. Terminating application.\n");
            free(config);
            return 1;
        }

        if (fread(config, sizeof(*config), 2000, cfg) == 0) {
            g_print("Looks like that the config file with the link to stream is missing. Terminating application.\n");
            fclose(cfg);
            free(config);
            return 1;
        }
        
        char *LOGIN = "LOGIN 1.8 %s %s";

        if (strlen(buffer) < 3 || !strncmp(buffer, "OK", 3)) {
            g_print("Login Failed");
        } else {
            if (!(pid = fork())) {
            // Streaming cam feed here.
            
            window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
            g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

            gtk_container_set_border_width(GTK_CONTAINER(window), 1);
    
            button = gtk_button_new_with_label("Open/close door!");
    
            g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(greet), "button");
    
            gtk_container_add(GTK_CONTAINER(window), button);
        
            gtk_widget_show_all(window);
            
            gtk_main();

            } else {
                       
                // Initialize GStreamer
                gst_init(&argc, &argv);
            
                // Build the pipeline
                pipeline = gst_parse_launch(config, NULL);
            
                // Start playing
                gst_element_set_state(pipeline, GST_STATE_PLAYING);
            
                // Wait until error or EOS
                bus = gst_element_get_bus(pipeline);
                msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
            }
        }
    }

    while (wait(NULL) != -1);

    // Free resources
    if (msg != NULL) {
        gst_message_unref(msg);
    }
    
    fclose(cfg);
    free(config);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}
