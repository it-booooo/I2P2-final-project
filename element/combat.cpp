#include <allegro5/allegro.h>
#include "combat.h"
#include "damageable.h"
#include "tree.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include "../scene/gamescene.h"
#include "../scene/sceneManager.h"
#include "../data/ImageCenter.h"

/*
   [Combat function]
*/

Elements *New_Combat(int label,
                     int x1, int y1,
                     int x2, int y2,
                     int damage, int side)
{
    Combat *entity = new Combat{};
    Elements *pObj = New_Elements(label);
    Elements &wrapper = *pObj;
    Combat   &obj = *entity;

    obj.img = ImageCenter::get_instance()->get("./assets/image/projectile.png");
    obj.width  = al_get_bitmap_width (obj.img);
    obj.height = al_get_bitmap_height(obj.img);
    obj.x1 = x1;
    obj.y1 = y1;
    obj.x2 = x2;
    obj.y2 = y2;
    obj.damage = damage;
    obj.side   = side;

    obj.hitbox = New_Rectangle(
        obj.x1,
        obj.y1,
        obj.x2,
        obj.y2
    );

    wrapper.inter_obj[wrapper.inter_len++] = tungtungtung_L;
    wrapper.inter_obj[wrapper.inter_len++] = Susu_L;
    wrapper.inter_obj[wrapper.inter_len++] = trippi_troppi_L;
    wrapper.inter_obj[wrapper.inter_len++] = capuccino_L;
    wrapper.inter_obj[wrapper.inter_len++] = bananini_L;
    wrapper.inter_obj[wrapper.inter_len++] = patapim_L;
    wrapper.inter_obj[wrapper.inter_len++] = tralala_L;
    wrapper.inter_obj[wrapper.inter_len++] = crocodilo_L;
    wrapper.inter_obj[wrapper.inter_len++] = bigtung_L;

    wrapper.entity   = entity;
    wrapper.Update   = Combat_update;
    wrapper.Interact = Combat_interact;
    wrapper.Draw     = Combat_draw;
    wrapper.Destroy  = Combat_destory;

    return pObj;
}

void Combat_update(Elements *self)
{
    _Combat_update_position(self, 0.0f, 0.0f);
}

void _Combat_update_position(Elements *self, float dx, float dy)
{
    Elements &wrapper = *self;
    Combat   &Obj = *static_cast<Combat *>(wrapper.entity);

    Obj.x1 += dx;
    Obj.x2 += dx;
    Obj.y1 += dy;
    Obj.y2 += dy;

    Shape *hitbox = Obj.hitbox;
    if (!hitbox) return;

    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);
}

void Combat_interact(Elements *self)
{
    Elements &wrapper = *self;
    Combat   &combat  = *static_cast<Combat *>(wrapper.entity);

    for (int j = 0; j < wrapper.inter_len; j++)
    {
        int inter_label = wrapper.inter_obj[j];
        ElementVec labelEle = sceneManager.GetLabelElements(inter_label);

        for (int i = 0; i < labelEle.len; i++)
        {
            Elements *tar = labelEle.arr[i];
            if (!tar || !tar->entity) continue;

            Damageable &target =
                *reinterpret_cast<Damageable *>(tar->entity);
            Shape *tar_hitbox = target.hitbox;

            if (!tar_hitbox || !combat.hitbox) continue;

            if (tar_hitbox->overlap(*combat.hitbox) &&
                combat.side != target.side)
            {
                DealDamageIfPossible(tar, combat.damage);
            }
        }
    }
    wrapper.dele = true;
}

void Combat_draw(Elements *self)
{
    Elements &wrapper = *self;
    Combat   &Obj = *static_cast<Combat *>(wrapper.entity);
    if (!Obj.img) return;

    if (Obj.x1 < Obj.x2)
        al_draw_bitmap(Obj.img, Obj.x1, Obj.y1, ALLEGRO_FLIP_HORIZONTAL);
    else
        al_draw_bitmap(Obj.img, Obj.x1, Obj.y1, 0);
}

void Combat_destory(Elements *self)
{
    if (!self || !self->entity) return;

    Combat *Obj = static_cast<Combat *>(self->entity);

    // projectile.png 由 ImageCenter 管理，不可 al_destroy_bitmap
    if (Obj->hitbox) {
        delete Obj->hitbox;
        Obj->hitbox = nullptr;
    }

    delete Obj;
    self->entity = nullptr;

    // 不要 delete/free self，交給 Scene / SceneManager 管
}
