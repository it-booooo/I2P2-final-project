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
#include "bloodman.h" 
#include "susu.h"

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

    // 超出邊界就刪
    if (atk.x < -atk.width || atk.x > DataCenter::WIDTH + atk.width ||
        atk.y < -atk.height || atk.y > DataCenter::HEIGHT + atk.height) {
        wrapper.dele = true;
        return;
    }

    // 先處理「環境」碰撞：Tree / Floor（不分陣營都會擋子彈）
    const int env_labels[] = { Tree_L, Floor_L };
    for (int j = 0; j < (int)(sizeof(env_labels)/sizeof(env_labels[0])); ++j) {
        ElementVec vec = sceneManager.GetLabelElements(env_labels[j]);
        for (int i = 0; i < vec.len; ++i) {
            Elements *tar_ptr = vec.arr[i];
            if (!tar_ptr || !tar_ptr->entity) continue;

            Damageable &tar = *reinterpret_cast<Damageable *>(tar_ptr->entity);
            if (!tar.hitbox || !atk.hitbox) continue;

            if (tar.hitbox->overlap(*atk.hitbox)) {
                wrapper.dele = true;
                return;
            }
        }
    }

    // =============================
    // 1) 敵方子彈：只打「當前操控角色」
    // =============================
    if (atk.side == SIDE_ENEMY) {
        Elements *cur = get_current_player();
        if (!cur || !cur->entity) return;

        Damageable &tar = *reinterpret_cast<Damageable *>(cur->entity);
        if (!tar.hitbox || !atk.hitbox) return;

        if (tar.hitbox->overlap(*atk.hitbox)) {
            DealDamageIfPossible(cur, atk.damage);
            wrapper.dele = true;
        }
        return;
    }

    // =============================
    // 2) 玩家子彈：掃描所有敵人 labels
    // =============================
    const int enemy_labels[] = {
        tungtungtung_L,
        trippi_troppi_L,
        capuccino_L,
        bananini_L,
        patapim_L,
        tralala_L,
        crocodilo_L,
        bigtung_L
        // 想打誰就加誰
    };

    for (int j = 0; j < (int)(sizeof(enemy_labels)/sizeof(enemy_labels[0])); ++j) {
        ElementVec vec = sceneManager.GetLabelElements(enemy_labels[j]);
        for (int i = 0; i < vec.len; ++i) {
            Elements *tar_ptr = vec.arr[i];
            if (!tar_ptr || !tar_ptr->entity) continue;

            Damageable &tar = *reinterpret_cast<Damageable *>(tar_ptr->entity);
            if (!tar.hitbox || !atk.hitbox) continue;

            if (!tar.hitbox->overlap(*atk.hitbox)) continue;

            // 只有玩家子彈打怪
            if (tar.side == SIDE_ENEMY) {
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
// 需要能用到 Bloodman / get_bloodman()

void DealDamageIfPossible(Elements *target, int damage)
{
    if (!target || !target->entity) return;

    // 先把 target 視為可被改寫的 realTarget（用來導正）
    Elements *realTarget = target;

    // 先拿到傳入 target 的 Damageable
    Damageable *td = reinterpret_cast<Damageable *>(realTarget->entity);
    if (!td || !td->hitbox) return;

    // ★導正：如果打到的是玩家(side==0)，就改成扣目前操控的角色
    // 這能修正「外部還在用 get_susu() 當 target」造成的扣血錯誤
    if (td->side == 0) {
        Elements *cur = get_current_player();
        if (cur && cur->entity && cur != realTarget) {
            Damageable *cd = reinterpret_cast<Damageable *>(cur->entity);
            if (cd && cd->hitbox && cd->side == 0) {
                realTarget = cur;
                td = cd; // ★同步換成 current player 的 Damageable
            }
        }
    }

    // 重新取得 Elements 參考（因為 realTarget 可能被換掉）
    Elements &tar = *realTarget;

    // ★ Bloodman 在 Q buff 期間：受擊傷害減半（只在「被打的人是 bloodman」時成立）
    Elements *bm_ele = get_bloodman();
    if (bm_ele == realTarget && bm_ele->entity)
    {
        Bloodman *bm = static_cast<Bloodman *>(bm_ele->entity);
        if (bm->lifesteal_active)
        {
            damage = (damage + 1) / 2; // ceil(damage/2)
        }
    }

    // 扣血
    td->hp -= damage;
    if (td->hp < 0) td->hp = 0;

    // 吸血：目標是怪(side==1) 才會幫 bloodman 補
    if (td->side == 1)
    {
        Elements *bm_ele2 = get_bloodman();
        if (bm_ele2 && bm_ele2->entity)
        {
            Bloodman *bm = static_cast<Bloodman *>(bm_ele2->entity);
            if (bm->lifesteal_active)
            {
                int heal = damage / 2;
                bm->base.hp += heal;
                if (bm->base.hp > bm->base.full_hp)
                    bm->base.hp = bm->base.full_hp;
            }
        }
    }

    // 死亡刪除
    if (td->hp <= 0) tar.dele = true;
}


