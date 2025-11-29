#include "tree.h"
#include "../shapes/Rectangle.h"
/*
   [tree function]
*/
Elements *New_Tree(int label)
{
    Tree *entity = static_cast<Tree *>(malloc(sizeof(Tree)));
    Elements *pObj = New_Elements(label);
    Elements &wrapper = *pObj;
    Tree &obj = *entity;

    obj.img = al_load_bitmap("assets/image/tree.png");
    obj.width = al_get_bitmap_width(obj.img);
    obj.height = al_get_bitmap_height(obj.img);
    obj.x = 85;
    obj.y = HEIGHT - obj.height;
    obj.base.hp=10;
    obj.base.hitbox = New_Rectangle(obj.x + obj.width / 3,
                                    obj.y + obj.height / 3,
                                    obj.x + 2 * obj.width / 3,
                                    obj.y + 2 * obj.height / 3);

    wrapper.entity = entity;
    wrapper.Update = Tree_update;
    wrapper.Interact = Tree_interact;
    wrapper.Draw = Tree_draw;
    wrapper.Destroy = Tree_destory;
    return pObj;
}
void Tree_update(Elements *self) {}
void Tree_interact(Elements *self) {}
void Tree_draw(Elements *self)
{
    Elements &wrapper = *self;
    Tree &Obj = *static_cast<Tree *>(wrapper.entity);
    al_draw_bitmap(Obj.img, Obj.x, Obj.y, 0);
}
void Tree_destory(Elements *self)
{
    Elements &wrapper = *self;
    Tree &Obj = *static_cast<Tree *>(wrapper.entity);
    al_destroy_bitmap(Obj.img);
    free(Obj.base.hitbox);
    free(&Obj);
    free(self);
}
