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
    Player * player = malloc(sizeof(player));

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
    magic   = gst_element_factory_make ("decodebin",       "magic");
    audio_sink     = gst_element_factory_make ("autoaudiosink", "audio-output");
    /*video_sink     = gst_element_factory_make ("autovideosink", "video-output");*/
    player->sink     = gst_element_factory_make ("xvimagesink", "video-output");
    player->subOverlay = NULL;
    // inutile puisque nous testerons uniquement si subOverlay est NULL
    player->subtitle = FALSE;

    if (!player->pipeline || !source || !magic || !audio_sink || !player->sink || !video_queue || !audio_queue) {
        g_printerr ("One element could not be created. Exiting.\n");
        return NULL;
    }

    /* Mise en place du pipeline */
    /* on configurer le nom du fichier a l'element source */
    g_object_set (G_OBJECT (source), "location", filename, NULL);


    /* on rajoute tous les elements dans le pipeline */
    /* file-source | ogg-demuxer | vorbis-video_queue | converter | alsa-output */
    gst_bin_add_many (GST_BIN (player->pipeline),
            source, magic, video_queue,audio_queue, player->sink, audio_sink , NULL);

    /* On relie les elements entre eux */
    /* file-source -> ogg-demuxer ~> vorbis-video_queue -> converter -> alsa-output */
    gst_element_link (source, magic);
    gst_element_link_many (audio_queue, audio_sink, NULL);
    gst_element_link_many (video_queue, player->sink, NULL);
    g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), audio_queue);
    g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), video_queue);



    return player;
}



Player * get_video_subtitle_player(const char * filename, const char * sub_name)
{

    GstElement *source, *magic, *video_queue,
               *audio_queue, *audio_sink, *sub_source,
               *parser;
    Player * player = malloc(sizeof(player));


    /* Creation des elements gstreamer */
    /* Notez que le demuxer va etre lie au decodeur dynamiquement.
       la raison est que Ogg peut contenir plusieurs flux (par exemple
       audio et video). Les connecteurs sources seront crees quand la
       lecture debutera, par le demuxer quand il detectera le nombre et
       la nature des flux. Donc nous connectons une fonction de rappel
       qui sera execute quand le "pad-added" sera emis. */
    // gst-launch
    player->pipeline = gst_pipeline_new ("audio-player");

    source   = gst_element_factory_make ("filesrc",       "file-source");
    magic   = gst_element_factory_make ("decodebin",       "magic");

    video_queue = gst_element_factory_make("queue", "video_queue");
    audio_queue = gst_element_factory_make("queue", "audio_queue");

    audio_sink     = gst_element_factory_make ("autoaudiosink", "audio-output");
    /*video_sink     = gst_element_factory_make ("autovideosink", "video-output");*/
    player->sink     = gst_element_factory_make ("xvimagesink", "video-output");

    /* Creation des elements pour les sous titres */
    sub_source = gst_element_factory_make("filesrc", "sub-source");
    parser = gst_element_factory_make("subparse", "parser");
    player->subOverlay = gst_element_factory_make("subtitleoverlay", "overlay");

    if (!player->pipeline || !source || !magic || !audio_sink || !player->sink || !video_queue || !audio_queue || !sub_source
            || !parser || !player->subOverlay) {
        g_printerr ("One element could not be created. Exiting.\n");
        return NULL;
    }

    /* Mise en place du pipeline */
    /* on configurer le nom du fichier a l'element source */
    g_object_set (G_OBJECT (source), "location", filename, NULL);

    g_object_set(G_OBJECT(sub_source), "location", sub_name, NULL);

    /* on rajoute tous les elements dans le pipeline */
    /* file-source | ogg-demuxer | vorbis-video_queue | converter | alsa-output */
    gst_bin_add_many (GST_BIN (player->pipeline),
            source, magic, video_queue,audio_queue,
            player->sink, audio_sink , sub_source, parser, player->subOverlay,NULL);

    /* On relie les elements entre eux */
    /* file-source -> ogg-demuxer ~> vorbis-video_queue -> converter -> alsa-output */
    gst_element_link (source, magic);
    gst_element_link_many (audio_queue, audio_sink, NULL);
    gst_element_link_many(sub_source,parser, player->subOverlay, NULL);
    gst_element_link_many (video_queue,player->subOverlay ,player->sink, NULL);
    //+ queue
    g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), audio_queue);

    g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), video_queue);

    /*g_signal_connect(parser, "pad-added", G_CALLBACK (on_pad_added), overlay);*/
    //gst_element_link_pads(parser,NULL, overlay,NULL);

    player->subtitle = TRUE;
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

void toogle_subtitle(Player * player)
{
    printf("youhou\n");
    printf("passed\n");
    if(player->subtitle)
        player->subtitle = FALSE;
    else {
        player->subtitle = TRUE;
    }
    printf("test\n");
    g_assert(G_IS_OBJECT(player->subOverlay));
    printf("youhou 2\n");
    if(player->subOverlay)
        printf("not null\n");
    else {
        printf("null\n");
    }
    g_object_set(G_OBJECT(player->subOverlay), "silent", player->subtitle, NULL);
    printf("youhou 3\n");
}

void start(Player *player, guintptr window_handle )
{

    GstBus *bus;
    /*SHOULD BE IN A FUNCTION BEGIN*/
    bus = gst_pipeline_get_bus (GST_PIPELINE (player->pipeline));
    /*gst_bus_add_watch (bus, bus_call, loop);*/
    gst_bus_add_watch (bus, bus_call, NULL);
    /*gst_bus_set_sync_handler (bus, (GstBusSyncHandler) bus_sync_handler, NULL);*/
    gst_object_unref (bus);

    /*
    GstElement * video_sink = sink;
    if (GST_IS_X_OVERLAY (video_sink))
    {
        gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (video_sink), GPOINTER_TO_INT (window_handle));
    }
    */
    gst_element_set_state (player->pipeline, GST_STATE_PLAYING);
}

void stop(Player * player)
{
    g_print ("Arret de la lecture\n");
    gst_element_set_state ((player->pipeline), GST_STATE_NULL);
}
