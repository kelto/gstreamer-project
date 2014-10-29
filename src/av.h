#ifndef AV_H
#define AV_H
#include <gst/gst.h>
#include <glib.h>

typedef struct _player
{
    GstElement * pipeline;
    GstElement * subOverlay;
    GstElement * sink;
    GstElement * volume;
    gboolean subtitle;
}Player;

Player * get_video_player(const char * filename);
int add_subtitle( GstElement * pipeline, const char * filename);
Player * get_video_subtitle_player(const char * filename, const char * sub_name);
void start(Player * player, guintptr window_handle );
void stop(Player * player);

#endif

