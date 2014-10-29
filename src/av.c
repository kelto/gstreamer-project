#include "av.h"
#include <gst/interfaces/xoverlay.h>
#include <string.h>


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

Player * get_video_player(const char * filename)
{

    GstElement  *source, *magic, *video_queue, *audio_queue,  *audio_sink;
    Player * player = malloc(sizeof(Player));

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
    magic   = gst_element_factory_make ("decodebin",       "magic");
    audio_sink     = gst_element_factory_make ("autoaudiosink", "audio-output");
    player->sink     = gst_element_factory_make ("xvimagesink", "video-output");
    player->subOverlay = NULL;
    // inutile puisque nous testerons uniquement si subOverlay est NULL

    if (!player->pipeline || !source || !magic || !player->volume || !audio_sink || !player->sink || !video_queue || !audio_queue) {
        g_printerr ("One element could not be created. Exiting.\n");
        return NULL;
    }

    /* Mise en place du pipeline */
    /* on configurer le nom du fichier a l'element source */
    g_object_set (G_OBJECT (source), "location", filename, NULL);


    /* on rajoute tous les elements dans le pipeline */
    /* file-source | ogg-demuxer | vorbis-video_queue | converter | alsa-output */
    gst_bin_add_many (GST_BIN (player->pipeline),
            source, magic, video_queue,audio_queue, player->volume, player->sink, audio_sink , NULL);

    /* On relie les elements entre eux */
    /* file-source -> ogg-demuxer ~> vorbis-video_queue -> converter -> alsa-output */
    gst_element_link (source, magic);
    gst_element_link_many (audio_queue, player->volume, audio_sink, NULL);
    gst_element_link_many (video_queue, player->sink, NULL);
    g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), audio_queue);
    g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), video_queue);

    player->silent = FALSE;

    return player;
}

Player * get_video_subtitle_player(const char * filename, const char * sub_name)
{

    GstElement  *source, *magic, *parser, *sub_source,
                *video_queue, *audio_queue,  *audio_sink;
    Player * player = malloc(sizeof(Player));

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
    audio_queue = gst_element_factory_make("queue", "audio_queue");
    player->volume = gst_element_factory_make("volume", "volume");
    player->subOverlay = gst_element_factory_make("subtitleoverlay", "subOverlay");
    sub_source = gst_element_factory_make("filesrc", "sub_source");
    parser = gst_element_factory_make("subparse", "parser");
    magic   = gst_element_factory_make ("decodebin",       "magic");
    audio_sink     = gst_element_factory_make ("autoaudiosink", "audio-output");
    player->sink     = gst_element_factory_make ("xvimagesink", "video-output");

    if (!player->pipeline || !source || !magic || !player->volume || !audio_sink || !player->subOverlay || !sub_source
            || !parser || !player->sink || !video_queue || !audio_queue) {
        g_printerr ("One element could not be created. Exiting.\n");
        return NULL;
    }

    /* Mise en place du pipeline */
    /* on configurer le nom du fichier a l'element source */
    g_object_set (G_OBJECT (source), "location", filename, NULL);
    g_object_set (G_OBJECT (sub_source), "location", sub_name, NULL);


    /* on rajoute tous les elements dans le pipeline */
    /* file-source | ogg-demuxer | vorbis-video_queue | converter | alsa-output */
    gst_bin_add_many (GST_BIN (player->pipeline),
            source, magic, video_queue,player->volume,
            audio_queue,sub_source, parser,player->subOverlay, player->sink, audio_sink , NULL);

    /* On relie les elements entre eux */
    /* file-source -> ogg-demuxer ~> vorbis-video_queue -> converter -> alsa-output */
    gst_element_link (source, magic);
    gst_element_link_many (audio_queue,player->volume, audio_sink, NULL);
    gst_element_link_many(sub_source, parser, player->subOverlay, NULL);
    gst_element_link_many (video_queue, player->subOverlay,player->sink, NULL);
    g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), audio_queue);
    g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), video_queue);


    player->silent = FALSE;
    return player;
}

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

   case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      /*g_main_loop_quit (loop);*/
      gtk_main_quit();
      break;
    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      /*g_main_loop_quit (loop);*/
      gtk_main_quit();
      break;
    }
    default:
      break;
  }
  return TRUE;
}

void start(Player *player, guintptr window_handle )
{

    GstBus *bus;
    /*SHOULD BE IN A FUNCTION BEGIN*/
    bus = gst_pipeline_get_bus (GST_PIPELINE (player->pipeline));
    gst_bus_add_watch (bus, bus_call, NULL);
    gst_object_unref (bus);

    gst_element_set_state (player->pipeline, GST_STATE_PLAYING);
}

void stop(Player * player)
{
    g_print ("Arret de la lecture\n");
    gst_element_set_state ((player->pipeline), GST_STATE_NULL);
}
