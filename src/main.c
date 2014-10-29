#include <gst/gst.h>
#include <glib.h>
#include "av.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gst/interfaces/xoverlay.h>

// voir comment mieux gerer l'id de l'handler
static guintptr window_handle = 0;

static void destroy(GtkWidget * widget, gpointer data)
{
    Player * player = (Player *) data;
    stop(player);
    gtk_main_quit();
}

static void realize(GtkWidget * video, gpointer data)
{
    GdkWindow * window = gtk_widget_get_window(video);
    window_handle = GDK_WINDOW_XID(window);
    Player * player = (Player *) data;
    if (GST_IS_X_OVERLAY (player->sink))
    {
        gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (player->sink), GPOINTER_TO_INT (window_handle));
    }

}

static void pause_cb(GtkButton * button, Player * player)
{
    gst_element_set_state ((player->pipeline), GST_STATE_PAUSED);
}
static void play_cb(GtkButton * button, Player * player)
{
    gst_element_set_state ((player->pipeline), GST_STATE_PLAYING);
}

static void get_ui(int argc, char * argv[], Player * player)
{
    GtkWidget * main_window,
              *video,
              *play,
              *pause,
              *subToogle,
              *subLabel,
              *hbox,
              *vbox;
    gtk_init(&argc, &argv);

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(main_window,"destroy", G_CALLBACK(destroy), player);

    video = gtk_drawing_area_new();
    gtk_widget_set_size_request(video,400,200);
    // mise en place d'un double buffer (eviter scintillement)
    gtk_widget_set_double_buffered(video,FALSE);
    /*gtk_container_add(GTK_CONTAINER(main_window),video);*/

    g_signal_connect(video,"realize", G_CALLBACK(realize),player);
    /*g_signal_connect(video,"expose_event", G_CALLBACK(expose_cb), NULL);*/

    play = gtk_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
    g_signal_connect(G_OBJECT(play),"clicked", G_CALLBACK(play_cb),player);

    pause = gtk_button_new_from_stock(GTK_STOCK_MEDIA_PAUSE);
    g_signal_connect(G_OBJECT(pause),"clicked", G_CALLBACK(pause_cb),player);

    subToogle = gtk_button_new();
    subLabel = gtk_label_new("subtitle");
    gtk_container_add(GTK_CONTAINER(subToogle), subLabel);
    g_signal_connect(G_OBJECT(subToogle), "clicked", G_CALLBACK(toogle_subtitle), player);

    vbox = gtk_vbox_new(FALSE,0);
    hbox = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox), video, TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hbox),play,FALSE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hbox),pause,FALSE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hbox),subToogle,FALSE,TRUE,0);


    gtk_container_add(GTK_CONTAINER(main_window),vbox);

    gtk_widget_show_all(main_window);


}

void set_start(gpointer data)
{
    Player * player = (Player *) data;
    start(player, window_handle);
}

int main (int argc, char *argv[])
{
    /*GMainLoop *loop;*/
    Player * player;

    /* Initialisation */
    gst_init (&argc, &argv);

    /*loop = g_main_loop_new (NULL, FALSE);*/


    /* Verification des arguments d'entree */
    switch(argc)
    {
        case 2:
            player = get_video_player(argv[1]);
            break;
        case 3:
            player = get_video_subtitle_player(argv[1], argv[2]);
            break;
        default:
            g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
            return -1;
    }

    get_ui(argc,argv, player);
    /* passage a l'etat "playing" du pipeline */
    g_print ("Lecture de : %s\n", argv[1]);
    /*g_idle_add(set_start,pipeline);*/
    /*gst_element_set_state (pipeline, GST_STATE_PLAYING);*/
    start(player, window_handle);

    /* Iteration */
    g_print ("En cours...\n");
    /*g_main_loop_run (loop);*/
    gtk_main();
    printf("stopped\n");

    /*SHOULD BE IN A FUNCTION END*/
    /* En dehors de la boucle principale, on nettoie proprement */
    gst_element_set_state ((player->pipeline), GST_STATE_NULL);
    g_print ("Suppression du pipeline\n");
    gst_object_unref(player->pipeline);
    /*free(player);*/

    return EXIT_SUCCESS;
}

