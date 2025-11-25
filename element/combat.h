#ifndef COMBAT_H_INCLUDED
#define COMBAT_H_INCLUDED
#include "element.h"
#include "../shapes/Shape.h"
/*
   [Combat object]
*/
typedef struct _Combat
{
    int x1, y1,x2,y2;          // the position of image
    int width, height; // the width and height of image
    //int vx,vy;             // the velocity of Combat
    int damage;
    int side;
    ALLEGRO_BITMAP *img;
    Shape *hitbox; // the hitbox of object
} Combat;
Elements *New_Combat(int label, int x1, int y1, int x2,int y2, int damage,int side);
void Combat_update(Elements *self);
void Combat_interact(Elements *self);
void Combat_draw(Elements *self);
void Combat_destory(Elements *self);
void _Combat_update_position(Elements *self, float dx, float dy);
void DealDamageIfPossible(Elements *target, int damage);

#endif