#define ALLEGRO_NO_MAGIC_MAIN
#include "GameWindow.h"
#include <time.h>
#include <stdlib.h>

int real_main(int argc, char **argv)
{
    srand((unsigned)time(NULL));

    Game *game = New_Game();
    game->execute(game);
    game->game_destroy(game);
	return 0;
}
int main(int argc, char **argv)
{
    return al_run_main(argc, argv, real_main);
}