#ifndef CAPUCCINO_H_INCLUDED
#define CAPUCCINO_H_INCLUDED

#include <allegro5/allegro_audio.h>
#include "element.h"
#include "damageable.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include <stdbool.h>
#include "state.h"

/* 新增狀態常數（不與 state.h 衝突） */
#define MUG 3

typedef struct
{
    Damageable      base;
    int             x, y;
    int             width, height;
    bool            dir;            // true: face right
    int             state;          // 0:STOP 1:MOVE 2:ATK 3:MUG
    ALLEGRO_BITMAP *img[4];         // 對應四種狀態
    int             anime, anime_time;
    bool            new_proj;
    int             attack_timer;   // 近戰冷卻 (frame)
    int             mug_timer;      // 還剩幾禎處於 mug
    int             mug_cooldown;   // 距離下次 mug 還有幾禎
} capuccino;

Elements *New_capuccino(int label);
void      capuccino_update(Elements *self);
void      capuccino_interact(Elements *self);
void      capuccino_draw(Elements *self);
void      capuccino_destory(Elements *self);
void      _capuccino_update_position(Elements *self, int dx, int dy);

#endif /* CAPUCCINO_H_INCLUDED */
