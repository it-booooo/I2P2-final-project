#include <allegro5/allegro.h>
#include "projectile.h"
#include "tree.h"
#include "../shapes/Circle.h"
#include "../shapes/ShapeFactory.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
#include <algorithm>
/*
   [Projectile function]
*/
Elements *New_Projectile(int label, int x, int y, int v)
{
    Projectile *entity = (Projectile *)malloc(sizeof(Projectile));
    Elements *pObj = New_Elements(label);
    // setting derived object member
    entity->img = al_load_bitmap("assets/image/projectile.png");
    entity->width = al_get_bitmap_width(entity->img);
    entity->height = al_get_bitmap_height(entity->img);
    entity->x = x;
    entity->y = y;
    entity->v = v;
    entity->hitbox = New_Circle(entity->x + entity->width / 2,
                                     entity->y + entity->height / 2,
                                     std::min(entity->width, entity->height) / 2);
    // setting the interact object
    pObj->inter_obj[pObj->inter_len++] = Tree_L;
    pObj->inter_obj[pObj->inter_len++] = Floor_L;
    // setting derived object function
    pObj->entity = entity;
    pObj->Update = Projectile_update;
    pObj->Interact = Projectile_interact;
    pObj->Draw = Projectile_draw;
    pObj->Destroy = Projectile_destory;

    return pObj;
}
void Projectile_update(Elements *self)
{
    Projectile *Obj = ((Projectile *)(self->entity));
    _Projectile_update_position(self, Obj->v, 0);
}
void _Projectile_update_position(Elements *self, int dx, int dy)
{
    Projectile *Obj = ((Projectile *)(self->entity));
    Obj->x += dx;
    Obj->y += dy;
    Shape *hitbox = Obj->hitbox;
    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);
}
void Projectile_interact(Elements *self)
{
    for (int j = 0; j < self->inter_len; j++)
    {
        int inter_label = self->inter_obj[j];
        ElementVec labelEle = _Get_label_elements(scene, inter_label);
        for (int i = 0; i < labelEle.len; i++)
        {
            if (inter_label == Floor_L)
            {
                _Projectile_interact_Floor(self, labelEle.arr[i]);
            }
            else if (inter_label == Tree_L)
            {
                _Projectile_interact_Tree(self, labelEle.arr[i]);
            }
        }
    }
}
void _Projectile_interact_Floor(Elements *self, Elements *tar)
{
    Projectile *Obj = ((Projectile *)(self->entity));
    if (Obj->x < 0 - Obj->width)
        self->dele = true;
    else if (Obj->x > WIDTH + Obj->width)
        self->dele = true;
}
void _Projectile_interact_Tree(Elements *self, Elements *tar)
{
    Projectile *Obj = ((Projectile *)(self->entity));
    Tree *tree = ((Tree *)(tar->entity));
    if (tree->base.hitbox && Obj->hitbox && tree->base.hitbox->overlap(*Obj->hitbox))
    {
        self->dele = true;
    }
}
void Projectile_draw(Elements *self)
{
    Projectile *Obj = ((Projectile *)(self->entity));
    if (Obj->v > 0)
        al_draw_bitmap(Obj->img, Obj->x, Obj->y, ALLEGRO_FLIP_HORIZONTAL);
    else
        al_draw_bitmap(Obj->img, Obj->x, Obj->y, 0);
}
void Projectile_destory(Elements *self)
{
    Projectile *Obj = ((Projectile *)(self->entity));
    al_destroy_bitmap(Obj->img);
    delete Obj->hitbox;
    free(Obj);
    free(self);
}
