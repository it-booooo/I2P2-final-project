#include <allegro5/allegro.h>
#include "earthquake.h"
#include "damageable.h"
#include "tree.h"
#include "../shapes/Circle.h"
#include "../shapes/ShapeFactory.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
#include <algorithm>
/*
   [Earthquake function]
*/
Elements *New_Earthquake(int label, int x, int y, int damage,int side)
{
    Earthquake *entity = (Earthquake *)malloc(sizeof(Earthquake));
    Elements *pObj = New_Elements(label);
    // setting derived object member
    entity->img = al_load_bitmap("assets/image/earthquake.png");
    entity->width = al_get_bitmap_width(entity->img);
    entity->height = al_get_bitmap_height(entity->img);
    entity->x = x;
    entity->y = y;
    entity->damage = damage;
    entity->side = side;
    entity->timer = 30;
    entity->hitbox = New_Circle(entity->x + entity->width / 2,
                                     entity->y + entity->height / 2,
                                     min(entity->width, entity->height) / 2);
    // setting the interact object
    pObj->inter_obj[pObj->inter_len++] = tungtungtung_L;
    pObj->inter_obj[pObj->inter_len++] = Susu_L;
    pObj->inter_obj[pObj->inter_len++] = trippi_troppi_L;
    //pObj->inter_obj[pObj->inter_len++] = capuccino_L;
    pObj->inter_obj[pObj->inter_len++] = patapim_L;
    pObj->inter_obj[pObj->inter_len++] = bananini_L;
    pObj->inter_obj[pObj->inter_len++] = tralala_L;
    pObj->inter_obj[pObj->inter_len++] = crocodilo_L;
    pObj->inter_obj[pObj->inter_len++] = bigtung_L;
    // setting derived object function
    pObj->entity = entity;
    pObj->Update = Earthquake_update;
    pObj->Interact = Earthquake_interact;
    pObj->Draw = Earthquake_draw;
    pObj->Destroy = Earthquake_destory;

    return pObj;
}
void Earthquake_update(Elements *self)
{
    Earthquake *earthquake = (Earthquake *)(self->entity);
    if(earthquake->timer>0) earthquake->timer --;
    else self->dele = true;
    _Earthquake_update_position(self, 0, 0);
}
void _Earthquake_update_position(Elements *self, float dx, float dy)
{
    /*Earthquake *Obj = ((Earthquake *)(self->entity));
    Obj->x1 += dx;
    Obj->x2 += dx;
    Obj->y1 += dy;
    Obj->y2 += dy;
    Shape *hitbox = Obj->hitbox;
    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);*/
}
void Earthquake_interact(Elements *self)
{
    Earthquake *earthquake = (Earthquake *)(self->entity);
    for (int j = 0; j < self->inter_len; j++)   //依序處理每種Label
    { 
        int inter_label = self->inter_obj[j];
        ElementVec labelEle = _Get_label_elements(scene, inter_label);
        for (int i = 0; i < labelEle.len; i++)  //依序處理同Label中的每個hitbox
        {
            Elements *tar = labelEle.arr[i];
            Shape *tar_hitbox = ((Damageable *)tar->entity)->hitbox;
            if (tar_hitbox && earthquake->hitbox && tar_hitbox->overlap(*earthquake->hitbox) && earthquake->side != ((Damageable *)tar->entity)->side) {
                DealDamageIfPossible(tar, earthquake->damage);
            }
        }
    }
    //self->dele = true;
}
void Earthquake_draw(Elements *self)
{
    Earthquake *Obj = ((Earthquake *)(self->entity));
    al_draw_bitmap(Obj->img,Obj->x,Obj->y, 0);
}
void Earthquake_destory(Elements *self)
{
    Earthquake *Obj = ((Earthquake *)(self->entity));
    al_destroy_bitmap(Obj->img);
    delete Obj->hitbox;
    free(Obj);
    free(self);
}
