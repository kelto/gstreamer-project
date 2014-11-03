#include "av.h"
#include <gst/interfaces/xoverlay.h>
#include <string.h>
#include <gtk/gtk.h>


static void on_pad_added (GstElement *element, GstPad *pad, gpointer    data)
{
    GstPad *sinkpad;
    GstElement *video_queue = (GstElement *) data;

    /* We can now link this pad with the vorbis-video_queue sink pad */
    g_print ("Dynamic pad created, linking demuxer/video_queue\n");

    sinkpad = gst_element_get_static_pad (video_queue, "sink");

    gst_pad_link (pad, sinkpad);
    gst_object_unref (sinkpad);
}

static GstElement * init_player(Player * player, const char * filename)
{
    GstElement  *source, *decode, *video_queue, *audio_queue,  *audio_sink;

    /* Creation des elements gstreamer */
    /* Notez que le demuxer va etre lie au decodeur dynamiquement.
       la raison est que Ogg peut contenir plusieurs flux (par exemple
       audio et video). Les connecteurs sources seront crees quand la
       lecture debutera, par le demuxer quand il detectera le nombre et
       la nature des flux. Donc nous connectons une fonction de rappel
       qui sera execute quand le "pad-added" sera emis. */
    player->pipeline = gst_pipeline_new ("audio-player");

    source   = gst_element_factory_make ("filesrc",       "file-source");
    video_queue = gst_element_factory_make("queue", "video_queue");
    player->volume = gst_element_factory_make("volume", "volume");
    audio_queue = gst_element_factory_make("queue", "audio_queue");
    decode   = gst_element_factory_make ("decodebin",       "decode");
    audio_sink     = gst_element_factory_make ("autoaudiosink", "audio-output");
    player->sink     = gst_element_factory_make ("xvimagesink", "video-output");
    player->subOverlay = NULL;

    if (!player->pipeline || !source || !decode || !player->volume || !audio_sink || !player->sink
            || !video_queue || !audio_queue) {
        g_printerr ("One element could not be created. Exiting.\n");
        return NULL;
    }

    /* on configurer le nom du fichier a l'element source */
    g_object_set (G_OBJECT (source), "location", filename, NULL);


    /* on rajoute tous les elements dans le pipeline */
    gst_bin_add_many (GST_BIN (player->pipeline),
            source, decode, video_queue,audio_queue, player->volume, player->sink, audio_sink , NULL);

    /* On relie les elements entre eux */
    gst_element_link (source, decode);
    gst_element_link_many (audio_queue, player->volume, audio_sink, NULL);
    g_signal_connect (decode, "pad-added", G_CALLBACK (on_pad_added), audio_queue);
    g_signal_connect (decode, "pad-added", G_CALLBACK (on_pad_added), video_queue);

    player->silent = FALSE;

    return video_queue;
}

int init_video_player(Player * player, const char * filename)
{

    GstElement * video_queue = init_player(player,filename);
    if(!video_queue)
    {
        g_printerr ("One element could not be created. Exiting.\n");
        return -1;
    }
    gst_element_link_many (video_queue, player->sink, NULL);

    return 0;
}

int init_video_player_subtitle(Player * player, const char * filename, const char * sub_name)
{

    GstElement   *parser, *sub_source, *video_queue;

    video_queue = init_player(player,filename);
    if(!video_queue)
    {
        g_printerr ("One element could not be created. Exiting.\n");
        return -1;
    }
    player->subOverlay = gst_element_factory_make("subtitleoverlay", "subOverlay");
    sub_source = gst_element_factory_make("filesrc", "sub_source");
    parser = gst_element_factory_make("subparse", "parser");

    if ( !player->subOverlay || !sub_source || !parser ) {
        g_printerr ("One element could not be created. Exiting.\n");
        return -1;
    }

    g_object_set (G_OBJECT (sub_source), "location", sub_name, NULL);


    /* on rajoute tous les elements dans le pipeline */
    gst_bin_add_many (GST_BIN (player->pipeline),
            sub_source, parser,player->subOverlay, NULL);

    /* On relie les elements entre eux */
    /* file-source -> ogg-demuxer ~> vorbis-video_queue -> converter -> alsa-output */
    gst_element_link_many(sub_source, parser, player->subOverlay, NULL);
    gst_element_link_many (video_queue, player->subOverlay,player->sink, NULL);

    player->silent = FALSE;
    return 0;
}



void player_play(Player * player)
{
    gst_element_set_state (player->pipeline, GST_STATE_PLAYING);
}

void player_pause(Player * player)
{
    gst_element_set_state (player->pipeline, GST_STATE_PAUSED);
}

void player_stop(Player * player)
{
    g_print ("Arret de la lecture\n");
    gst_element_set_state ((player->pipeline), GST_STATE_NULL);
}
static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{

  switch (GST_MESSAGE_TYPE (msg)) {

   case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      gtk_main_quit();
      break;
    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      gtk_main_quit();
      break;
    }
    default:
      break;
  }
  return TRUE;
}

void player_start(Player *player)
{

    GstBus *bus;
    bus = gst_pipeline_get_bus (GST_PIPELINE (player->pipeline));
    gst_bus_add_watch (bus, bus_call, NULL);
    gst_object_unref (bus);
    player_play(player);
}
