#ifndef FLOOR_H_INCLUDED
#define FLOOR_H_INCLUDED
#include "element.h"
/*
   [floor object]
*/
class Floor : public Element
{
public:
    int x, y;          // the position of image
    int width, height; // the width and height of image
    ALLEGRO_BITMAP *img;
    int map_data[6][6];
};

Elements *New_Floor(int label);
void Floor_update(Elements *self);
void Floor_interact(Elements *self);
void Floor_draw(Elements *self);
void Floor_destory(Elements *self);
void _Floor_load_map(Floor *floor);
void _Floor_interact_Character(Elements *self, Elements *tar);

#endif
