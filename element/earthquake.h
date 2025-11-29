#ifndef EARTHQUAKE_H_INCLUDED
#define EARTHQUAKE_H_INCLUDED
#include "element.h"
#include "../shapes/Shape.h"
/*
   [Earthquake
 object]
*/
typedef struct _Earthquake
{
    int x,y;          // the position of image
    int width, height; // the width and height of image
    //int vx,vy;             // the velocity of Earthquake
    int damage;
    int side;
    int timer;
    ALLEGRO_BITMAP *img;
    Shape *hitbox; // the hitbox of object
} Earthquake;
Elements *New_Earthquake(int label, int x, int y, int damage,int side);
void Earthquake_update(Elements *self);
void Earthquake_interact(Elements *self);
void Earthquake_draw(Elements *self);
void Earthquake_destory(Elements *self);
void _Earthquake_update_position(Elements *self, float dx, float dy);
void DealDamageIfPossible(Elements *target, int damage);

#endif