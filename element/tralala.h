#ifndef tralala_H_INCLUDED
#define tralala_H_INCLUDED

#include <allegro5/allegro_audio.h>
#include "element.h"
#include "damageable.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include <stdbool.h>
#include "state.h"

/*
   [tralala object]
*/
typedef struct
{
    Damageable base;
    int  x, y;
    int  width, height;             // image size
    bool dir;                       // true: face right
    int  state;                     // STOP / MOVE / ATTACK
    ALLEGRO_BITMAP *img[3];         // 0:stop 1:move 2:attack
    int  anime;                     // (若日後要做動圖可留)
    int  anime_time;
    bool new_proj;                  // 目前沒用，可保留
    int  attack_timer;              // ★ 個別冷卻計時器 (frame)
    int quake_timer;
} tralala;

Elements *New_tralala(int label);
void      tralala_update(Elements *self);
void      tralala_interact(Elements *self);
void      tralala_draw(Elements *self);
void      tralala_destory(Elements *self);
void      _tralala_update_position(Elements *self, int dx, int dy);

#endif /* tralala_H_INCLUDED */
