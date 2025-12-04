#include "atk.h"
#include "damageable.h"
#include "tree.h"
#include "../shapes/Circle.h"
#include "../shapes/ShapeFactory.h"
#include "../scene/gamescene.h"
#include "../scene/sceneManager.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>

/* 陣營常數 */
#define SIDE_PLAYER 0
#define SIDE_ENEMY  1

/* --------------------------------------------------
 * New_Atk：建立一顆子彈
 * --------------------------------------------------*/
Elements *New_Atk(int label,
                  int x, int y,
                  float vx, float vy,
                  int damage, int side)
{
    Atk *entity  = new Atk{};
    Elements *pObj = New_Elements(label);
    Elements &obj = *pObj;
    Atk &atk = *entity;

    /* 預設貼圖由 ImageCenter 管理 */
    atk.img = ImageCenter::get_instance()->get("./assets/image/fire_ball.png");

    atk.width  = al_get_bitmap_width(atk.img);
    atk.height = al_get_bitmap_height(atk.img);
    atk.x      = x;
    atk.y      = y;
    atk.vx     = vx;
    atk.vy     = vy;
    atk.damage = damage;
    atk.side   = side;

    atk.hitbox = New_Circle(
        atk.x + atk.width  / 2,
        atk.y + atk.height / 2,
        (float)(fmin(atk.width, atk.height)) / 2.0f
    );

    /* 碰撞白名單 */
    obj.inter_obj[obj.inter_len++] = Tree_L;
    obj.inter_obj[obj.inter_len++] = Floor_L;
    obj.inter_obj[obj.inter_len++] = tungtungtung_L;
    obj.inter_obj[obj.inter_len++] = Susu_L;
    obj.inter_obj[obj.inter_len++] = trippi_troppi_L;
    obj.inter_obj[obj.inter_len++] = capuccino_L;
    obj.inter_obj[obj.inter_len++] = bananini_L;
    obj.inter_obj[obj.inter_len++] = patapim_L;
    obj.inter_obj[obj.inter_len++] = tralala_L;
    obj.inter_obj[obj.inter_len++] = crocodilo_L;
    obj.inter_obj[obj.inter_len++] = bigtung_L;

    /* 綁定函式 */
    obj.entity   = entity;
    obj.Update   = Atk_update;
    obj.Interact = Atk_interact;
    obj.Draw     = Atk_draw;
    obj.Destroy  = Atk_destory;

    return pObj;
}

/* --------------------------------------------------
 * 在擊發後隨時換子彈貼圖
 * --------------------------------------------------*/
void Atk_set_image(Elements *self, const char *img_path)
{
    if (!self || !img_path) return;
    Atk &obj = *static_cast<Atk *>(self->entity);

    /* 所有圖片都必須交給 ImageCenter 管，不可手動 destroy */
    obj.img = ImageCenter::get_instance()->get(img_path);
    if (!obj.img) return;

    obj.width  = al_get_bitmap_width(obj.img);
    obj.height = al_get_bitmap_height(obj.img);

    if (obj.hitbox) delete obj.hitbox;
    obj.hitbox = New_Circle(
        obj.x + obj.width  / 2,
        obj.y + obj.height / 2,
        (float)(fmin(obj.width, obj.height)) / 2.0f
    );
}

/* --------------------------------------------------
 * Update
 * --------------------------------------------------*/
void Atk_update(Elements *self)
{
    Atk &obj = *static_cast<Atk *>(self->entity);
    obj.x += obj.vx;
    obj.y += obj.vy;

    Shape *hit = obj.hitbox;
    if (!hit) return;

    const double cx = hit->center_x();
    const double cy = hit->center_y();
    hit->update_center_x(cx + obj.vx);
    hit->update_center_y(cy + obj.vy);
}

/* --------------------------------------------------
 * Interact
 * --------------------------------------------------*/
void Atk_interact(Elements *self)
{
    Elements &wrapper = *self;
    Atk &atk = *static_cast<Atk *>(self->entity);

    if (atk.x < -atk.width || atk.x > DataCenter::WIDTH + atk.width) {
        wrapper.dele = true;
        return;
    }

    for (int j = 0; j < wrapper.inter_len; ++j) {
        ElementVec vec = sceneManager.GetLabelElements(wrapper.inter_obj[j]);

        for (int i = 0; i < vec.len; ++i) {
            Elements *tar_ptr = vec.arr[i];
            if (!tar_ptr || !tar_ptr->entity) continue;

            Damageable &tar = *reinterpret_cast<Damageable *>(tar_ptr->entity);
            Shape *tar_hit = tar.hitbox;

            if (!tar_hit || !atk.hitbox) continue;
            if (!tar_hit->overlap(*atk.hitbox)) continue;

            if ((atk.side == SIDE_PLAYER && tar.side == SIDE_ENEMY) ||
                (atk.side == SIDE_ENEMY  && tar.side == SIDE_PLAYER))
            {
                DealDamageIfPossible(tar_ptr, atk.damage);
                wrapper.dele = true;
                return;
            }
        }
    }
}

/* --------------------------------------------------
 * Draw
 * --------------------------------------------------*/
void Atk_draw(Elements *self)
{
    Atk &obj = *static_cast<Atk *>(self->entity);
    if (!obj.img) return;

    int flags = (obj.vx > 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    al_draw_bitmap(obj.img, obj.x, obj.y, flags);
}

/* --------------------------------------------------
 * Destroy
 * --------------------------------------------------*/
void Atk_destory(Elements *self)
{
    if (!self || !self->entity) return;

    Atk *obj = static_cast<Atk *>(self->entity);

    /* hitbox 需要釋放 */
    delete obj->hitbox;
    obj->hitbox = nullptr;

    /* 整個 Atk 物件釋放 */
    delete obj;
    self->entity = nullptr;

    /*  不可 free(self)！Elements* 由 SceneManager 管 */
}

/* --------------------------------------------------
 * Damage helper
 * --------------------------------------------------*/
void DealDamageIfPossible(Elements *target, int damage)
{
    if (!target || !target->entity) return;

    Damageable &dmg = *reinterpret_cast<Damageable *>(target->entity);
    if (!dmg.hitbox) return;

    dmg.hp -= damage;
    if (dmg.hp <= 0) target->dele = true;
}
