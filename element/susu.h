#ifndef SUSU_H_INCLUDED
#define SUSU_H_INCLUDED
#include <allegro5/allegro_audio.h>
#include "damageable.h"
#include "element.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include "state.h"
#include <stdbool.h>

/*
   [susu object]
*/
/*typedef enum susuType
{
    STOP = 0,
    MOVE,
    ATK
} susuType;*/
typedef struct _susu
{
    Damageable base;
    int x, y;
    int width, height;              // the width and height of image
    int dir;                       // 1: face to right, 0: face to left, 2:face to up, 3:face to down
    int damage;
    int state;                      // the state of susu
    ALGIF_ANIMATION *gif_status[5]; // gif for each state. 0: stop, 1: move, 2:attack
    //ALLEGRO_BITMAP *img;
    ALLEGRO_SAMPLE_INSTANCE *atk_Sound;
    int anime;      // counting the time of animation
    int anime_time; // indicate how long the animation
    int e_timer,q_timer;
    bool new_proj;
    //Shape *hitbox; // the hitbox of object
} susu;
Elements *New_susu(int label);
void susu_update(Elements *self);
void susu_interact(Elements *self);
void susu_draw(Elements *self);
void susu_destroy(Elements *self);
void _susu_update_position(Elements *self, int dx, int dy);
Elements * get_susu(void);

#endif
