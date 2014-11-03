#include <gst/gst.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "av.h"
#include "ui.h"


int main (int argc, char *argv[])
{
    Player player;

    /* Initialisation */
    gst_init (&argc, &argv);
    gtk_init(&argc, &argv);

    int rc = 0;
    /* Verification des arguments d'entree */
    switch(argc)
    {
        case 2:
            rc = init_video_player(&player,argv[1]);
            break;
        case 3:
            rc = init_video_player_subtitle(&player,argv[1],argv[2]);
            break;
        default:
            g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
            return -1;
    }
    if(rc)
    {
        g_printerr ("Player could not be created. Exiting.\n");
        return -1;
    }
    get_ui(&player);
    /* passage a l'etat "playing" du pipeline */
    g_print ("Lecture de : %s\n", argv[1]);
    player_start(&player);


    /* Iteration */
    g_print ("En cours...\n");
    /*g_main_loop_run (loop);*/
    gtk_main();

    /*SHOULD BE IN A FUNCTION END*/
    /* En dehors de la boucle principale, on nettoie proprement */
    gst_element_set_state ((player.pipeline), GST_STATE_NULL);
    g_print ("Suppression du pipeline\n");
    gst_object_unref(player.pipeline);

    return EXIT_SUCCESS;
}

