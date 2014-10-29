#include <gst/gst.h>
#include <glib.h>
static gboolean bus_call (GstBus     *bus,
                          GstMessage *msg,
                          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

   case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }
  return TRUE;
}

static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (decoder, "sink");

  gst_pad_link (pad, sinkpad);
  gst_object_unref (sinkpad);
}

int main (int   argc,
          char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *magic, *video_queue, *audio_queue, *video_sink, *audio_sink;
  GstBus *bus;

  /* Initialisation */
  gst_init (&argc, &argv);

 loop = g_main_loop_new (NULL, FALSE);

  /* Verification des arguments d'entree */
  if (argc != 2) {
    g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
    return -1;
  }

  /* Creation des elements gstreamer */
  pipeline = gst_pipeline_new ("audio-player");
  source   = gst_element_factory_make ("filesrc",       "file-source");
  video_queue = gst_element_factory_make("queue", "video_queue");
  audio_queue = gst_element_factory_make("queue", "audio_queue");
  magic   = gst_element_factory_make ("decodebin",       "magic");
  audio_sink     = gst_element_factory_make ("autoaudiosink", "audio-output");
  video_sink     = gst_element_factory_make ("autovideosink", "video-output");

  if (!pipeline || !source || !magic || !audio_sink || !video_sink || !video_queue || !audio_queue) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Mise en place du pipeline */
  /* on configurer le nom du fichier a l'element source */
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);

  /* on rajoute une gestion de messages */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* on rajoute tous les elements dans le pipeline */
  /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, magic, video_queue,audio_queue, video_sink, audio_sink , NULL);

  /* On relie les elements entre eux */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
  gst_element_link (source, magic);
  gst_element_link_many (audio_queue, audio_sink, NULL);
  gst_element_link_many (video_queue, video_sink, NULL);
  g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), audio_queue);
  g_signal_connect (magic, "pad-added", G_CALLBACK (on_pad_added), video_queue);

  /* Notez que le demuxer va etre lie au decodeur dynamiquement.
     la raison est que Ogg peut contenir plusieurs flux (par exemple
     audio et video). Les connecteurs sources seront crees quand la
     lecture debutera, par le demuxer quand il detectera le nombre et
     la nature des flux. Donc nous connectons une fonction de rappel
     qui sera execute quand le "pad-added" sera emis. */

  /* passage a l'etat "playing" du pipeline */
  g_print ("Lecture de : %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* Iteration */
  g_print ("En cours...\n");
  g_main_loop_run (loop);

  /* En dehors de la boucle principale, on nettoie proprement */
  g_print ("Arret de la lecture\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("Suppression du pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  return 0;
}

