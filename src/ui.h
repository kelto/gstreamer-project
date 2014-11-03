#ifndef UI_H
#define UI_H
/* argc, argv : les
void get_ui(int argc, char * argv[], Player * player);
*/
/* player : pointeur vers le Player qui sera utilise pour la lecture.
 * Initialise et lance l'interface graphique.
 * Attention : gtk_init doit etre lance au prealable.
 */
void get_ui( Player * player);

#endif

