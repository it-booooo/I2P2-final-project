#include "Ball.h"
#include "tree.h"
#include "charater.h"
#include "../shapes/Circle.h"
#include "../shapes/ShapeFactory.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
#include "../data/DataCenter.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_primitives.h"
/*
   [Ball function]
*/
Elements *New_Ball(int label)
{
    Ball *entity = static_cast<Ball *>(malloc(sizeof(Ball)));
    Elements *pObj = New_Elements(label);
    Elements &wrapper = *pObj;
    Ball &obj = *entity;
    DataCenter *DC = DataCenter::get_instance();

    obj.x = DC->mouse.x;
    obj.y = DC->mouse.y;
    obj.r = 10;
    obj.color = al_map_rgb(255 ,0, 0);
    obj.hitbox = New_Circle(obj.x,
                             obj.y,
                             obj.r);

    wrapper.inter_obj[wrapper.inter_len++] = Character_L;
    wrapper.inter_obj[wrapper.inter_len++] = Tree_L;

    wrapper.entity = entity;
    wrapper.Update = Ball_update;
    wrapper.Interact = Ball_interact;
    wrapper.Draw = Ball_draw;
    wrapper.Destroy = Ball_destory;

    return pObj;
}
void Ball_update(Elements *self)
{
    Elements &wrapper = *self;
    Ball &Obj = *static_cast<Ball *>(wrapper.entity);
    Shape *hitbox = Obj.hitbox;
    DataCenter *DC = DataCenter::get_instance();
    if (hitbox) {
        const double cx = hitbox->center_x();
        const double cy = hitbox->center_y();
        hitbox->update_center_x(cx + DC->mouse.x - Obj.x);
        hitbox->update_center_y(cy + DC->mouse.y - Obj.y);
    }
    Obj.x = DC->mouse.x;
    Obj.y = DC->mouse.y;
}
void _Ball_update_position(Elements *self, int dx, int dy)
{
    Elements &wrapper = *self;
    Ball &Obj = *static_cast<Ball *>(wrapper.entity);
    Obj.x += dx;
    Obj.y += dy;
    Shape *hitbox = Obj.hitbox;
    if (!hitbox) return;

    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);
}
void Ball_interact(Elements *self)
{
    Elements &wrapper = *self;
    Ball &Obj = *static_cast<Ball *>(wrapper.entity);
    for (int j = 0; j < wrapper.inter_len; j++)
    {
        ElementVec labelEle = sceneManager.GetLabelElements(Character_L);
        Character *Obj1 = NULL;
        if (labelEle.len > 0 && labelEle.arr[0]) {
            Elements &chara_wrapper = *labelEle.arr[0];
            Obj1 = static_cast<Character *>(chara_wrapper.entity);
        }
        labelEle = sceneManager.GetLabelElements(Tree_L);
        Tree *Obj2 = NULL;
        if (labelEle.len > 0 && labelEle.arr[0]) {
            Elements &tree_wrapper = *labelEle.arr[0];
            Obj2 = reinterpret_cast<Tree *>(tree_wrapper.entity);
        }

        Shape *chara_hitbox = NULL;
        if (Obj1) {
            Character &chara = *Obj1;
            chara_hitbox = chara.hitbox;
        }
        if(Obj.hitbox && chara_hitbox)
        {
            Shape &ball_hitbox = *Obj.hitbox;
            if (ball_hitbox.overlap(*chara_hitbox)) {
                Obj.color = al_map_rgb(0,255,0);
            }
        }
        Shape *tree_hitbox = NULL;
        if (Obj2) {
            Tree &tree = *Obj2;
            tree_hitbox = tree.base.hitbox;
        }
        else if(Obj.hitbox && tree_hitbox)
        {
            Shape &ball_hitbox = *Obj.hitbox;
            if (ball_hitbox.overlap(*tree_hitbox)) {
                Obj.color = al_map_rgb(0,0,255);
            }
        }
        else
        {
            Obj.color = al_map_rgb(255,0,0);
        }
    }
}
void _Ball_interact_Floor(Elements *self, Elements *tar)
{
}
void _Ball_interact_Tree(Elements *self, Elements *tar)
{
}
void Ball_draw(Elements *self)
{
    Elements &wrapper = *self;
    Ball &Obj = *static_cast<Ball *>(wrapper.entity);
    al_draw_circle(Obj.x,Obj.y,Obj.r,Obj.color,10);
}
void Ball_destory(Elements *self)
{
    Elements &wrapper = *self;
    Ball &Obj = *static_cast<Ball *>(wrapper.entity);
    delete Obj.hitbox;
    free(&Obj);
    free(self);
}
