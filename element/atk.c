#include "Atk.h"
#include "damageable.h"
#include "tree.h"
#include "../shapes/Circle.h"
#include "../scene/gamescene.h"
#include "../scene/sceneManager.h"

#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
    Atk      *pDerivedObj = malloc(sizeof(Atk));
    Elements *pObj        = New_Elements(label);

    /* 預設貼圖 */
    pDerivedObj->img = al_load_bitmap("assets/image/fire_ball.png");

    pDerivedObj->width  = al_get_bitmap_width (pDerivedObj->img);
    pDerivedObj->height = al_get_bitmap_height(pDerivedObj->img);
    pDerivedObj->x      = x;
    pDerivedObj->y      = y;
    pDerivedObj->vx     = vx;
    pDerivedObj->vy     = vy;
    pDerivedObj->damage = damage;
    pDerivedObj->side   = side;

    pDerivedObj->hitbox = New_Circle(
        pDerivedObj->x + pDerivedObj->width  / 2,
        pDerivedObj->y + pDerivedObj->height / 2,
        (float)fmin(pDerivedObj->width, pDerivedObj->height) / 2);

    /* 碰撞白名單（保持原來順序） */
    pObj->inter_obj[pObj->inter_len++] = Tree_L;
    pObj->inter_obj[pObj->inter_len++] = Floor_L;
    pObj->inter_obj[pObj->inter_len++] = tungtungtung_L;
    pObj->inter_obj[pObj->inter_len++] = Susu_L;
    pObj->inter_obj[pObj->inter_len++] = trippi_troppi_L;
    pObj->inter_obj[pObj->inter_len++] = capuccino_L;
    pObj->inter_obj[pObj->inter_len++] = bananini_L;
    pObj->inter_obj[pObj->inter_len++] = patapim_L;
    pObj->inter_obj[pObj->inter_len++] = tralala_L;
    pObj->inter_obj[pObj->inter_len++] = crocodilo_L;
    pObj->inter_obj[pObj->inter_len++] = bigtung_L;

    /* 綁定函式 */
    pObj->pDerivedObj = pDerivedObj;
    pObj->Update      = Atk_update;
    pObj->Interact    = Atk_interact;
    pObj->Draw        = Atk_draw;
    pObj->Destroy     = Atk_destory;

    return pObj;
}

/* --------------------------------------------------
 * ★ 在擊發後隨時換子彈貼圖
 * --------------------------------------------------*/
void Atk_set_image(Elements *self, const char *img_path)
{
    if (!self || !self->pDerivedObj || !img_path) return;

    Atk *Obj = self->pDerivedObj;

    if (Obj->img) al_destroy_bitmap(Obj->img);
    Obj->img = al_load_bitmap(img_path);
    if (!Obj->img) {
        fprintf(stderr, "Atk_set_image(): 無法載入 %s\n", img_path);
        return;
    }

    /* 更新幾何與 hitbox */
    Obj->width  = al_get_bitmap_width (Obj->img);
    Obj->height = al_get_bitmap_height(Obj->img);

    if (Obj->hitbox) free(Obj->hitbox);
    Obj->hitbox = New_Circle(Obj->x + Obj->width / 2,
                             Obj->y + Obj->height / 2,
                             (float)fmin(Obj->width, Obj->height) / 2);
}

/* ------------------------- Update ------------------------- */
void Atk_update(Elements *self)
{
    Atk *Obj = self->pDerivedObj;
    _Atk_update_position(self, Obj->vx, Obj->vy);
}

void _Atk_update_position(Elements *self, float dx, float dy)
{
    Atk *Obj = self->pDerivedObj;
    Obj->x += dx;
    Obj->y += dy;
    Obj->hitbox->update_center_x(Obj->hitbox, dx);
    Obj->hitbox->update_center_y(Obj->hitbox, dy);
}

/* ------------------------ Interact ------------------------ */
void Atk_interact(Elements *self)
{
    Atk *atk = self->pDerivedObj;

    /* 超出場景刪除 */
    if (atk->x < -atk->width || atk->x > WIDTH + atk->width) {
        self->dele = true;
        return;
    }

    for (int j = 0; j < self->inter_len; ++j) {
        ElementVec vec = _Get_label_elements(scene, self->inter_obj[j]);

        for (int i = 0; i < vec.len; ++i) {
            Elements *tar     = vec.arr[i];
            Shape    *tar_hit = ((Damageable *)tar->pDerivedObj)->hitbox;

            if (!tar_hit) continue;
            if (!tar_hit->overlap(tar_hit, atk->hitbox)) continue;

            int bullet_side = atk->side;
            int target_side = ((Damageable *)tar->pDerivedObj)->side;

            /* 只允許 0 ↔ 1 互打，其它不扣血 */
            if ((bullet_side == SIDE_PLAYER && target_side == SIDE_ENEMY) ||
                (bullet_side == SIDE_ENEMY  && target_side == SIDE_PLAYER))
            {
                DealDamageIfPossible(tar, atk->damage);
                self->dele = true;
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
    Atk *Obj = self->pDerivedObj;
    if (!Obj->img) return;

    int flags = (Obj->vx > 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    al_draw_bitmap(Obj->img, Obj->x, Obj->y, flags);
}

/* ----------------------- Destroy -------------------------- */
void Atk_destory(Elements *self)
{
    Atk *Obj = self->pDerivedObj;
    if (Obj->img)    al_destroy_bitmap(Obj->img);
    if (Obj->hitbox) free(Obj->hitbox);
    free(Obj);
    free(self);
}

/* ---------------- Damage helper (原碼) -------------------- */
void DealDamageIfPossible(Elements *target, int damage)
{
    if (!target || !target->pDerivedObj) return;
    Damageable *dmg = target->pDerivedObj;
    if (!dmg->hitbox) return;

    dmg->hp -= damage;
    if (dmg->hp <= 0) target->dele = true;
}
