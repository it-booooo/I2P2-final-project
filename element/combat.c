#include "combat.h"
#include "damageable.h"
#include "tree.h"
#include "../shapes/Rectangle.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
/*
   [Combat function]
*/
Elements *New_Combat(int label, int x1, int y1, int x2,int y2, int damage,int side)
{
    Combat *pDerivedObj = (Combat *)malloc(sizeof(Combat));
    Elements *pObj = New_Elements(label);
    // setting derived object member
    pDerivedObj->img = al_load_bitmap("assets/image/projectile.png");
    pDerivedObj->width = al_get_bitmap_width(pDerivedObj->img);
    pDerivedObj->height = al_get_bitmap_height(pDerivedObj->img);
    pDerivedObj->x1 = x1;
    pDerivedObj->y1 = y1;
    pDerivedObj->x2 = x2;
    pDerivedObj->y2 = y2;
    pDerivedObj->damage = damage;
    pDerivedObj->side = side;
    pDerivedObj->hitbox = New_Rectangle(pDerivedObj->x1,
                                        pDerivedObj->y1,
                                        pDerivedObj->x2,
                                        pDerivedObj->y2);
    // setting the interact object
    pObj->inter_obj[pObj->inter_len++] = tungtungtung_L;
    pObj->inter_obj[pObj->inter_len++] = Susu_L;
    pObj->inter_obj[pObj->inter_len++] = trippi_troppi_L;
    pObj->inter_obj[pObj->inter_len++] = capuccino_L;
    pObj->inter_obj[pObj->inter_len++] = bananini_L;
    pObj->inter_obj[pObj->inter_len++] = patapim_L;
    pObj->inter_obj[pObj->inter_len++] = tralala_L;
    pObj->inter_obj[pObj->inter_len++] = crocodilo_L;
    pObj->inter_obj[pObj->inter_len++] = bigtung_L;
    // setting derived object function
    pObj->pDerivedObj = pDerivedObj;
    pObj->Update = Combat_update;
    pObj->Interact = Combat_interact;
    pObj->Draw = Combat_draw;
    pObj->Destroy = Combat_destory;

    return pObj;
}
void Combat_update(Elements *self)
{
    _Combat_update_position(self, 0, 0);
}
void _Combat_update_position(Elements *self, float dx, float dy)
{
    Combat *Obj = ((Combat *)(self->pDerivedObj));
    Obj->x1 += dx;
    Obj->x2 += dx;
    Obj->y1 += dy;
    Obj->y2 += dy;
    Shape *hitbox = Obj->hitbox;
    hitbox->update_center_x(hitbox, dx);
    hitbox->update_center_y(hitbox, dy);
}
void Combat_interact(Elements *self)
{
    Combat *combat = (Combat *)(self->pDerivedObj);
    for (int j = 0; j < self->inter_len; j++)   //依序處理每種Label
    { 
        int inter_label = self->inter_obj[j];
        ElementVec labelEle = _Get_label_elements(scene, inter_label);
        for (int i = 0; i < labelEle.len; i++)  //依序處理同Label中的每個hitbox
        {
            Elements *tar = labelEle.arr[i];
            Shape *tar_hitbox = ((Damageable *)tar->pDerivedObj)->hitbox;
            if (tar_hitbox && tar_hitbox->overlap(tar_hitbox, combat->hitbox) && combat->side != ((Damageable *)tar->pDerivedObj)->side) {
                DealDamageIfPossible(tar, combat->damage);
            }
        }
    }
    self->dele = true;
}
void Combat_draw(Elements *self)
{
    Combat *Obj = ((Combat *)(self->pDerivedObj));
    if (Obj->x1 < Obj->x2)
        al_draw_bitmap(Obj->img, Obj->x1, Obj->y1, ALLEGRO_FLIP_HORIZONTAL);
    else
        al_draw_bitmap(Obj->img, Obj->x1, Obj->y1, 0);
}
void Combat_destory(Elements *self)
{
    Combat *Obj = ((Combat *)(self->pDerivedObj));
    al_destroy_bitmap(Obj->img);
    free(Obj->hitbox);
    free(Obj);
    free(self);
}
