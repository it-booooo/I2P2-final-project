#ifndef bigtung_H_INCLUDED
#define bigtung_H_INCLUDED

#include <allegro5/allegro_audio.h>
#include "element.h"
#include "damageable.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include <stdbool.h>
#include "state.h"

/*
   [bigtung object]
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
} bigtung;

Elements *New_bigtung(int label);
void      bigtung_update(Elements *self);
void      bigtung_interact(Elements *self);
void      bigtung_draw(Elements *self);
void      bigtung_destory(Elements *self);
void      _bigtung_update_position(Elements *self, int dx, int dy);

#endif /* bigtung_H_INCLUDED */
