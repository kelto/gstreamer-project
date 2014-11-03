#include <gst/gst.h>
#include <glib.h>
#include "av.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gst/interfaces/xoverlay.h>


static void destroy(GtkWidget * widget, gpointer data)
{
    Player * player = (Player *) data;
    player_stop(player);
    gtk_main_quit();
}

static void realize(GtkWidget * video, gpointer data)
{
    GdkWindow * window = gtk_widget_get_window(video);
    guintptr window_handle = GDK_WINDOW_XID(window);
    Player * player = (Player *) data;
    if (GST_IS_X_OVERLAY (player->sink))
    {
        gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (player->sink), GPOINTER_TO_INT (window_handle));
    }

}

static void pause_cb(GtkButton * button, Player * player)
{
    player_pause(player);
}

static void play_cb(GtkButton * button, Player * player)
{
    player_play(player);
}

static void subtitle_cb(GtkButton * button, Player * player)
{
    player->silent = ! player->silent;
    g_object_set(G_OBJECT(player->subOverlay), "silent", player->silent, NULL);
}

static void volume_cb(GtkRange * range, Player * player)
{
    gdouble value = gtk_range_get_value(range)/100;
    g_object_set(G_OBJECT(player->volume), "volume", value, NULL);
}

void get_ui( Player * player)
{
    GtkWidget * main_window,
              *video,
              *play,
              *pause,
              *subToogle,
              *subLabel,
              *volume_slider,
              *hbox,
              *vbox;

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(main_window,"destroy", G_CALLBACK(destroy), player);

    video = gtk_drawing_area_new();
    gtk_widget_set_size_request(video,400,200);
    // mise en place d'un double buffer (eviter scintillement)
    gtk_widget_set_double_buffered(video,FALSE);

    g_signal_connect(video,"realize", G_CALLBACK(realize),player);
    /*g_signal_connect(video,"expose_event", G_CALLBACK(expose_cb), NULL);*/

    play = gtk_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
    g_signal_connect(G_OBJECT(play),"clicked", G_CALLBACK(play_cb),player);

    pause = gtk_button_new_from_stock(GTK_STOCK_MEDIA_PAUSE);
    g_signal_connect(G_OBJECT(pause),"clicked", G_CALLBACK(pause_cb),player);




    volume_slider = gtk_hscale_new_with_range(0,100,1);
    gtk_range_set_value(GTK_RANGE(volume_slider),100);

    g_signal_connect(G_OBJECT(volume_slider), "value-changed", G_CALLBACK(volume_cb), player);

    vbox = gtk_vbox_new(FALSE,0);
    hbox = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox), video, TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(vbox), volume_slider, FALSE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hbox),play,FALSE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hbox),pause,FALSE,TRUE,0);
    if(player->subOverlay)
    {
        subToogle = gtk_button_new();
        g_signal_connect(G_OBJECT(subToogle), "clicked", G_CALLBACK(subtitle_cb), player);
        subLabel = gtk_label_new("subtitle");
        gtk_container_add(GTK_CONTAINER(subToogle), subLabel);
        gtk_box_pack_start(GTK_BOX(hbox),subToogle,FALSE,TRUE,0);
    }

    gtk_container_add(GTK_CONTAINER(main_window),vbox);

    gtk_widget_show_all(main_window);


}

