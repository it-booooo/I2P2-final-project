#ifndef BALL_H_INCLUDED
#define BALL_H_INCLUDED

#include "element.h"
#include "../shapes/Shape.h"
#include <allegro5/allegro.h>   // for ALLEGRO_COLOR

/*
   [Ball object]
*/
class Ball : public Element
{
public:
    int x, y;              // position
    int r;                 // radius
    Shape *hitbox;         // hitbox
    ALLEGRO_COLOR color;   // draw color

    Ball()
        : x(0), y(0), r(10),
          hitbox(nullptr),
          color(al_map_rgb(255, 0, 0))
    {}
};

Elements *New_Ball(int label);
void Ball_update(Elements *self);
void Ball_interact(Elements *self);
void Ball_draw(Elements *self);

// ✅ 統一命名：destroy
void Ball_destroy(Elements *self);

// ✅ 相容舊拼字（如果其他檔案還在用 Ball_destory）
inline void Ball_destory(Elements *self) { Ball_destroy(self); }

#endif
