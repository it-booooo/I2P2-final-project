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

/* -------- 陣營常數（僅 0 / 1，避免誤傳） -------- */
#define SIDE_PLAYER 0
#define SIDE_ENEMY  1

/* --------------------------------------------------
 * New_Atk ：建立一顆子彈（預設圖 ball-02.png，可之後呼叫
 *            Atk_set_image() 換成任意 .png/.jpg）
 * --------------------------------------------------*/
Elements *New_Atk(int label,
                  int x, int y,
                  float vx, float vy,
                  int damage, int side)
{
    Atk      *entity = static_cast<Atk *>(malloc(sizeof(Atk)));
    Elements *pObj   = New_Elements(label);
    Elements &obj    = *pObj;
    Atk &atk         = *entity;

    /* 預設貼圖 */
    atk.img = al_load_bitmap("assets/image/fire_ball.png");

    atk.width  = al_get_bitmap_width (atk.img);
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
        (float)fmin(atk.width, atk.height) / 2);

    /* 碰撞白名單（保持原來順序） */
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
    obj.entity     = entity;
    obj.Update     = Atk_update;
    obj.Interact   = Atk_interact;
    obj.Draw       = Atk_draw;
    obj.Destroy    = Atk_destory;

    return pObj;
}

/* --------------------------------------------------
 * ★ 在擊發後隨時換子彈貼圖
 * --------------------------------------------------*/
void Atk_set_image(Elements *self, const char *img_path)
{
    if (!self) return;
    Elements &wrapper = *self;
    if (!wrapper.entity || !img_path) return;

    Atk &obj = *static_cast<Atk *>(wrapper.entity);

    if (obj.img) al_destroy_bitmap(obj.img);
    obj.img = al_load_bitmap(img_path);
    if (!obj.img) {
        fprintf(stderr, "Atk_set_image(): 無法載入 %s\n", img_path);
        return;
    }

    /* 更新幾何與 hitbox */
    obj.width  = al_get_bitmap_width (obj.img);
    obj.height = al_get_bitmap_height(obj.img);

    if (obj.hitbox) delete obj.hitbox;
    obj.hitbox = New_Circle(obj.x + obj.width / 2,
                             obj.y + obj.height / 2,
                             (float)fmin(obj.width, obj.height) / 2);
}

/* ------------------------- Update ------------------------- */
void Atk_update(Elements *self)
{
    Elements &wrapper = *self;
    Atk &obj = *static_cast<Atk *>(wrapper.entity);
    _Atk_update_position(self, obj.vx, obj.vy);
}

void _Atk_update_position(Elements *self, float dx, float dy)
{
    Elements &wrapper = *self;
    Atk &obj = *static_cast<Atk *>(wrapper.entity);
    obj.x += dx;
    obj.y += dy;
    Shape *hit = obj.hitbox;
    if (!hit) return;

    const double cx = hit->center_x();
    const double cy = hit->center_y();
    hit->update_center_x(cx + dx);
    hit->update_center_y(cy + dy);
}

/* ------------------------ Interact ------------------------ */
void Atk_interact(Elements *self)
{
    Elements &wrapper = *self;
    Atk &atk = *static_cast<Atk *>(wrapper.entity);

    /* 超出場景刪除 */
    if (atk.x < -atk.width || atk.x > DataCenter::WIDTH + atk.width) {
        wrapper.dele = true;
        return;
    }

    for (int j = 0; j < wrapper.inter_len; ++j) {
        ElementVec vec = sceneManager.GetLabelElements(wrapper.inter_obj[j]);

        for (int i = 0; i < vec.len; ++i) {
            Elements *tar_ptr = vec.arr[i];
            if (!tar_ptr) continue;

            Elements &tar = *tar_ptr;
            if (!tar.entity) continue;

            Damageable &target_entity = *reinterpret_cast<Damageable *>(tar.entity);
            Shape *tar_hit = target_entity.hitbox;

            if (!tar_hit || !atk.hitbox) continue;
            if (!tar_hit->overlap(*atk.hitbox)) continue;

            int bullet_side = atk.side;
            int target_side = target_entity.side;

            /* 只允許 0 ↔ 1 互打，其它不扣血 */
            if ((bullet_side == SIDE_PLAYER && target_side == SIDE_ENEMY) ||
                (bullet_side == SIDE_ENEMY  && target_side == SIDE_PLAYER))
            {
                DealDamageIfPossible(tar_ptr, atk.damage);
                wrapper.dele = true;
                return;
            }
        }
    }
}

/* -------------------------------------------------- 其他私用互動（未改動，可留空或保留原碼） */
void _Atk_interact_Floor(Elements *self, Elements *tar) { /* ...若需要可保留... */ }
void _Atk_interact_Tree (Elements *self, Elements *tar) { /* ...若需要可保留... */ }

/* ------------------------- Draw --------------------------- */
void Atk_draw(Elements *self)
{
    Elements &wrapper = *self;
    Atk &obj = *static_cast<Atk *>(wrapper.entity);
    if (!obj.img) return;

    int flags = (obj.vx > 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    al_draw_bitmap(obj.img, obj.x, obj.y, flags);
}

/* ----------------------- Destroy -------------------------- */
void Atk_destory(Elements *self)
{
    Elements &wrapper = *self;
    Atk &obj = *static_cast<Atk *>(wrapper.entity);
    if (obj.img)    al_destroy_bitmap(obj.img);
    if (obj.hitbox) delete obj.hitbox;
    free(wrapper.entity);
    free(self);
}

/* ---------------- Damage helper (原碼) -------------------- */
void DealDamageIfPossible(Elements *target, int damage)
{
    if (!target) return;
    Elements &tar = *target;
    if (!tar.entity) return;

    Damageable &dmg = *reinterpret_cast<Damageable *>(tar.entity);
    if (!dmg.hitbox) return;

    dmg.hp -= damage;
    if (dmg.hp <= 0) tar.dele = true;
}
