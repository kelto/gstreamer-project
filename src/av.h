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
    gboolean silent;
}Player;

/* return : retourne 0 si le Player a bien ete initialise,
 * -1 en cas d'erreurs.
 * player : pointeur vers une structure Player
 * filename : le nom du fichier video qui sera lu
 * Cette fonction va creer tous les elements du pipeline necessaire
 * a la lecture avec gstreamer du fichier video, et les lier.
 */
int init_video_player(Player * player, const char * filename);
/* return : retourne 0 si le Player a bien ete initialise,
 * -1 en cas d'erreurs.
 * player : pointeur vers une structure Player
 * filename : le nom du fichier video qui sera lu
 * sub-name : le nom du fichier des sous-titres
 * Cette fonction va creer tous les elements du pipeline necessaire
 * a la lecture avec gstreamer du fichier video avec sa piste de
 * sous-titres, et les lier.
 */
int init_video_player_subtitle(Player * player, const char * filename, const char * sub_name);
/* player : le pointeur du Player a lancer
 * window_handle :
 * Lance la lecture du player.
 */
void player_start(Player * player);
/* player : le pointeur du Player à lancer
 * active la lecture. Attention cette fonction n'est pas a appeler si
 * la fonction player_start n'a pas deja ete lance.
 */
void player_play(Player * player);
/* player : le pointeur vers le Player a stopper.
 * Met fin a la lecture du Player. Cette fonction ne devrait etre
 * appeler que lorsque l'on souhaite mettre fin au programme.
 */
void player_stop(Player * player);
/*player : pointeur vers le Player a stopper.
 * Met en pause la lecture du player.
 * La lecture peut etre reprise avec player_play.
 */
void player_pause(Player * player);

#endif

