#ifndef HPBAR_H_INCLUDED
#define HPBAR_H_INCLUDED
#include "element.h"
#include "../scene/gamescene.h" // for element label
#include "../shapes/Shape.h"
#include "allegro5/allegro_ttf.h"
/*
   [Hpbar object]
*/
typedef struct _Hpbar
{
    int x, y;          // the position of image
    int full_length;
    int full_hp,now_hp;
    ALLEGRO_FONT *font;
    //Shape *hitbox; // the hitbox of object
    ALLEGRO_COLOR color;
} Hpbar;
Elements *New_Hpbar(int label,int full_hp, int now_hp);
void Hpbar_update(Elements *self);
void Hpbar_interact(Elements *self);
void Hpbar_draw(Elements *self);
void Hpbar_destroy(Elements *self);

#endif
