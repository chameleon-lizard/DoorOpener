#include <gst/gst.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>

enum
{
    URL_MAX = 2000
};

static int isOpened = 0;

void greet(GtkWidget *widget, gpointer data )
{
    g_print("Door is %s!\n", isOpened ? "opened" : "closed");
    isOpened = !isOpened;
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
    char *address = calloc(2000, sizeof(*address));
    
    FILE *config = 0;
 
    int pid = 0;

    if (!(pid = fork())) {
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
        /* Let's set the border width of the window to 20.
         * You may play with the value and see the
         * difference. */
        gtk_container_set_border_width(GTK_CONTAINER(window), 20);

        button = gtk_button_new_with_label("Open/close door!");

        g_signal_connect(GTK_OBJECT(button), "clicked", G_CALLBACK(greet), "button");

        gtk_container_add(GTK_CONTAINER(window), button);
    
        gtk_widget_show_all(window);
    
        gtk_main();
    } else {
        if (!(config = fopen("config", "r"))) {
            printf("Config file not found. Terminating application.\n");
            free(address);
            return 1;
        }
    
        if (fread(address, sizeof(*address), 2000, config) == 0) {
            printf("Looks like that the config file with the link to stream is missing. Terminating application.\n");
            fclose(config);
            free(address);
            return 1;
        }
    
        /* Initialize GStreamer */
        gst_init(&argc, &argv);
    
        /* Build the pipeline */
        pipeline = gst_parse_launch(address, NULL);
    
        /* Start playing */
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    
        /* Wait until error or EOS */
        bus = gst_element_get_bus(pipeline);
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    }

    while (wait(NULL) != -1);

    /* Free resources */
    if (msg != NULL) {
        gst_message_unref(msg);
    }
    
    fclose(config);
    free(address);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}
