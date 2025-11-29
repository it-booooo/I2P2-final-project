#ifndef patapim_H_INCLUDED
#define patapim_H_INCLUDED

#include <allegro5/allegro_audio.h>
#include "element.h"
#include "damageable.h"
#include "state.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include <stdbool.h>

/*
   [patapim object]
*/
typedef struct {
    Damageable base;
    int  x, y;
    int  width, height;
    bool dir;
    int  state;
    ALLEGRO_BITMAP *img[3];
    int  anime;
    int  anime_time;
    bool new_proj;
    int  attack_timer;
    int  quake_timer; // ★ 新增：地震攻擊冷卻時間
} patapim;

Elements *New_patapim(int label);
void      patapim_update(Elements *self);
void      patapim_interact(Elements *self);
void      patapim_draw(Elements *self);
void      patapim_destory(Elements *self);
void      _patapim_update_position(Elements *self, int dx, int dy);

#endif /* patapim_H_INCLUDED */