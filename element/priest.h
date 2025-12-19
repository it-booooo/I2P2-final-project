#ifndef PRIEST_H_INCLUDED
#define PRIEST_H_INCLUDED

#include <allegro5/allegro_audio.h>
#include "damageable.h"
#include "element.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include "state.h"

// 全域目前操控的是哪一個角色：1 = susu, 2 = bloodman, 3 = priest
extern int gControlledCharacter;

/*
   [priest object]  // ★ 第三位角色：牧師
*/
class priest : public Element
{
public:
    Damageable base;
    int x, y;
    int width, height;              // the width and height of image
    int dir;                       // 1: face to right, 0: face to left, 2:face to up, 3:face to down
    int damage;
    int state;                      // the state of priest
    ALGIF_ANIMATION *gif_status[5]; // gif for each state. 0: stop, 1: move, 2:attack
    ALLEGRO_SAMPLE_INSTANCE *atk_Sound;
    int anime;      // counting the time of animation
    int anime_time; // indicate how long the animation
    int e_timer,q_timer;
    bool new_proj;
};

Elements *New_priest(int label);
void priest_update(Elements *self);
void priest_interact(Elements *self);
void priest_draw(Elements *self);
void priest_destroy(Elements *self);
void _priest_update_position(Elements *self, int dx, int dy);

Elements *get_priest(void);

#endif
