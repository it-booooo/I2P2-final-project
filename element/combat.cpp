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
    Combat *entity = static_cast<Combat *>(malloc(sizeof(Combat)));
    Elements *pObj = New_Elements(label);
    Elements &wrapper = *pObj;
    Combat &obj = *entity;

    obj.img = al_load_bitmap("assets/image/projectile.png");
    obj.width = al_get_bitmap_width(obj.img);
    obj.height = al_get_bitmap_height(obj.img);
    obj.x1 = x1;
    obj.y1 = y1;
    obj.x2 = x2;
    obj.y2 = y2;
    obj.damage = damage;
    obj.side = side;
    obj.hitbox = New_Rectangle(obj.x1,
                               obj.y1,
                               obj.x2,
                               obj.y2);

    wrapper.inter_obj[wrapper.inter_len++] = tungtungtung_L;
    wrapper.inter_obj[wrapper.inter_len++] = Susu_L;
    wrapper.inter_obj[wrapper.inter_len++] = trippi_troppi_L;
    wrapper.inter_obj[wrapper.inter_len++] = capuccino_L;
    wrapper.inter_obj[wrapper.inter_len++] = bananini_L;
    wrapper.inter_obj[wrapper.inter_len++] = patapim_L;
    wrapper.inter_obj[wrapper.inter_len++] = tralala_L;
    wrapper.inter_obj[wrapper.inter_len++] = crocodilo_L;
    wrapper.inter_obj[wrapper.inter_len++] = bigtung_L;

    wrapper.entity = entity;
    wrapper.Update = Combat_update;
    wrapper.Interact = Combat_interact;
    wrapper.Draw = Combat_draw;
    wrapper.Destroy = Combat_destory;

    return pObj;
}
void Combat_update(Elements *self)
{
    _Combat_update_position(self, 0, 0);
}
void _Combat_update_position(Elements *self, float dx, float dy)
{
    Elements &wrapper = *self;
    Combat &Obj = *static_cast<Combat *>(wrapper.entity);
    Obj.x1 += dx;
    Obj.x2 += dx;
    Obj.y1 += dy;
    Obj.y2 += dy;
    Shape *hitbox = Obj.hitbox;
    if (!hitbox) return;

    Shape &box = *hitbox;
    box.update_center_x(&box, dx);
    box.update_center_y(&box, dy);
}
void Combat_interact(Elements *self)
{
    Elements &wrapper = *self;
    Combat &combat = *static_cast<Combat *>(wrapper.entity);
    for (int j = 0; j < wrapper.inter_len; j++)   //依序處理每種Label
    {
        int inter_label = wrapper.inter_obj[j];
        ElementVec labelEle = _Get_label_elements(scene, inter_label);
        for (int i = 0; i < labelEle.len; i++)  //依序處理同Label中的每個hitbox
        {
            Elements *tar = labelEle.arr[i];
            if (!tar) continue;

            Elements &target_wrapper = *tar;
            if (!target_wrapper.entity) continue;

            Damageable &target = *static_cast<Damageable *>(target_wrapper.entity);
            Shape *tar_hitbox = target.hitbox;
            if (tar_hitbox && combat.hitbox) {
                Shape &tar_box = *tar_hitbox;
                if (tar_box.overlap(&tar_box, combat.hitbox) && combat.side != target.side) {
                    DealDamageIfPossible(tar, combat.damage);
                }
            }
        }
    }
    wrapper.dele = true;
}
void Combat_draw(Elements *self)
{
    Elements &wrapper = *self;
    Combat &Obj = *static_cast<Combat *>(wrapper.entity);
    if (Obj.x1 < Obj.x2)
        al_draw_bitmap(Obj.img, Obj.x1, Obj.y1, ALLEGRO_FLIP_HORIZONTAL);
    else
        al_draw_bitmap(Obj.img, Obj.x1, Obj.y1, 0);
}
void Combat_destory(Elements *self)
{
    Elements &wrapper = *self;
    Combat &Obj = *static_cast<Combat *>(wrapper.entity);
    al_destroy_bitmap(Obj.img);
    free(Obj.hitbox);
    free(&Obj);
    free(self);
}
