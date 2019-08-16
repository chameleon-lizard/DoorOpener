#include <gst/video/videooverlay.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gdk/gdk.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <wait.h>

enum
{
    RETURN_VALUE_SIZE = 1024,
    RETURN_LENGTH = 3,
    DISPLACEMENT = 5,
    CFG_MAX = 2000,
    PORT = 3312,
    ERROR = 3,
    START = 0,
    QUIT = 2,
    OPEN = 1
};

int valread;

char *sip;

// Structure to contain all our information, so we can pass it around
typedef struct _CustomData {
    GstElement *playbin;           // Our one and only pipeline

    GtkWidget *slider;              // Slider widget to keep track of current position 
    GtkWidget *streams_list;        // Text widget to display info about the streams 
    gulong slider_update_signal_id; // Signal ID for the slider update signal 

    GstState state;                 // Current state of the pipeline 
    gint64 duration;                // Duration of the clip, in nanoseconds 
} CustomData;

static void 
realize_cb(GtkWidget *widget, CustomData *data) 
{
    GdkWindow *window = gtk_widget_get_window(widget);
    guintptr window_handle;

    window_handle = GDK_WINDOW_XID(window);

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(data->playbin), window_handle);
}

// This function is called when the PLAY button is clicked
static void 
dooropen(GtkButton *button, CustomData *data) 
{
    system("ssh -t opener@skud762.bch.cs.msu.ru \"./server-komodo\"");
}

// This function is called when the main window is closed
static void 
delete_event_cb(GtkWidget *widget, GdkEvent *event, CustomData *data) 
{
    gtk_main_quit();
}

static void 
create_ui(CustomData *data) 
{
    GtkWidget *main_window;  // The uppermost window, containing all other windows 
    GtkWidget *video_window; // The drawing area where the video will be shown 
    GtkWidget *main_box;     // VBox to hold main_hbox and the controls 
    GtkWidget *main_hbox;    // HBox to hold the video_window 
    GtkWidget *controls;     // HBox to hold the buttons and the slider 
    GtkWidget *open_button;  // Buttons 

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(main_window), "delete-event", G_CALLBACK(delete_event_cb), data);

    video_window = gtk_drawing_area_new();
    g_signal_connect(video_window, "realize", G_CALLBACK(realize_cb), data);

    open_button = gtk_button_new_from_icon_name("go-next", GTK_ICON_SIZE_SMALL_TOOLBAR);
    g_signal_connect(G_OBJECT(open_button), "clicked", G_CALLBACK(dooropen), data);

    controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(controls), open_button, FALSE, FALSE, 2);

    main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(main_hbox), video_window, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_hbox), data->streams_list, FALSE, FALSE, 2);

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), main_hbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), controls, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window), main_box);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1280, 750);

    gtk_widget_show_all(main_window);
}

int 
main(int argc, char *argv[]) 
{
    // Reading the opener IP address, login and password from the config
    FILE *config = 0;
    if (!(config = fopen("config", "r"))) {
        printf("Error opening config. Is it created?\n");
        return -1;
    }

    char *cfg = (char *)calloc(CFG_MAX, sizeof(*cfg));
    
    fread(cfg, CFG_MAX, sizeof(*cfg), config);
    fclose(config);
    
    sip = DISPLACEMENT + strstr(cfg, "SIP:");
    *strstr(sip, "\n") = 0;

    CustomData data;
    GstStateChangeReturn ret;
    GstBus *bus;
    
    // Initialize GTK
    gtk_init(&argc, &argv);
    
    // Initialize GStreamer
    gst_init(&argc, &argv);
    
    memset(&data, 0, sizeof(data));
    data.duration = GST_CLOCK_TIME_NONE;
    
    data.playbin = gst_element_factory_make("playbin", "playbin");
    
    if (!data.playbin) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }
    
    g_object_set(data.playbin, "uri", sip, NULL);
    
    create_ui(&data);
    
    ret = gst_element_set_state(data.playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(data.playbin);
        return -1;
    }

    // Start the GTK main loop. We will not regain control until gtk_main_quit is called.
    gtk_main();

    // Free resources
    gst_element_set_state(data.playbin, GST_STATE_NULL);
    gst_object_unref(data.playbin);

    free(cfg);

    return 0;
}
