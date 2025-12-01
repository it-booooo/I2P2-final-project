#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "tungtungtung.h"
#include "susu.h"              /* 提供 get_susu() 介面 */
#include "combat.h"            /* 近戰攻擊矩形 */
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h" /* sceneManager.RegisterElement & Combat_L */
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>


/* --------------------------------------------------
   追蹤／攻擊參數
   --------------------------------------------------*/
#define CHASE_SPEED            3.0f   /* 像素 / frame */
#define ARRIVE_EPSILON         60.0f  /* 抵達判定半徑 */
#define ATTACK_DISTANCE        150.0f /* 自動攻擊距離 */
#define ATTACK_COOLDOWN_FRAMES 120    /* 2 秒冷卻 (60 FPS * 2) */
#define TUNG_ATTACK_DAMAGE    30     /* ← 在這裡調整攻擊力 */

/* --------------------------------------------------
   建構函式
   --------------------------------------------------*/
Elements *New_tungtungtung(int label)
{
    tungtungtung *entity = static_cast<tungtungtung *>(malloc(sizeof(tungtungtung)));
    Elements     *pObj        = New_Elements(label);

    /* 載入靜態貼圖 */
    const char *state_string[3] = {"stop", "move", "atk"};
    for (int i = 0; i < 3; ++i) {
        char buffer[64];
        sprintf(buffer, "assets/image/tungtungtung_%s.png", state_string[i]);
        entity->img[i] = al_load_bitmap(buffer);
    }

    /* 幾何資料 */
    entity->width  = al_get_bitmap_width (entity->img[0]);
    entity->height = al_get_bitmap_height(entity->img[0]);
    entity->x = 300;
    entity->y = DataCenter::HEIGHT - entity->height - 60;
    entity->base.hp   = 50;
    entity->base.side = 1;          /* 敵方陣營 */
    

    /* 個別冷卻計時器初始化 */
    entity->attack_timer = 0;

    /* 避開出生點太靠近玩家 */
    Elements *susu_elem = get_susu();
    susu *player = NULL;
    if (susu_elem) player = (susu *)susu_elem->entity;
    do {
        entity->x = rand() % (DataCenter::WIDTH  - entity->width);
        entity->y = rand() % (DataCenter::HEIGHT - entity->height);
    } while (player && fabs(entity->x - player->x) < ARRIVE_EPSILON &&
                       fabs(entity->y - player->y) < ARRIVE_EPSILON);

    /* 依最終座標建立 hitbox */
    entity->base.hitbox = New_Rectangle(entity->x,
                                        entity->y,
                                        entity->x + entity->width,
                                        entity->y + entity->height);

    entity->dir   = false;  /* 預設面向左 */
    entity->state = STOP;

    /* 綁定多型函式 */
    pObj->entity = entity;
    pObj->Draw        = tungtungtung_draw;
    pObj->Update      = tungtungtung_update;
    pObj->Interact    = tungtungtung_interact;
    pObj->Destroy     = tungtungtung_destory;

    return pObj;
}

/* --------------------------------------------------
   每幀更新：固定速率追蹤 susu，近距離自動攻擊
   --------------------------------------------------*/
void tungtungtung_update(Elements *self)
{
    tungtungtung *chara = static_cast<tungtungtung *>(self->entity);

    /* 攻擊冷卻倒數 */
    if (chara->attack_timer > 0) chara->attack_timer--;

    /* 透過單例 accessor 取得 susu */
    Elements *susu_elem = get_susu();
    if (!susu_elem) return;              /* 還沒生成 susu */

    susu *target = static_cast<susu *>(susu_elem->entity);

    /* 1) 取得雙方中心點 */
    float cx = chara->x + chara->width  * 0.5f;
    float cy = chara->y + chara->height * 0.5f;
    float tx = target->x + target->width  * 0.5f;
    float ty = target->y + target->height * 0.5f;

    float dx = tx - cx;
    float dy = ty - cy;
    float dist = sqrtf(dx * dx + dy * dy);

    /* 2) 追蹤移動或停止 */
    if (dist > ARRIVE_EPSILON) {
        float vx = CHASE_SPEED * dx / dist;
        float vy = CHASE_SPEED * dy / dist;
        _tungtungtung_update_position(self, (int)vx, (int)vy);
        chara->dir   = (dx >= 0);
        if (chara->state != ATK) chara->state = MOVE;
    } else {
        if (chara->state != ATK) chara->state = STOP;
    }

    /* 3) 自動攻擊判定 */
    if (dist <= ATTACK_DISTANCE && chara->attack_timer == 0) {
        /* 近戰矩形寬 / 長 */
        const int reach = 120;  /* 延伸距離 */
        const int thick = 200;  /* 攻擊寬度 */

        /* 決定攻擊方向 */
        int dir;
        if (fabsf(dx) > fabsf(dy)) {
            dir = (dx >= 0) ? 0 : 3; /* 0:右, 3:左 */
        } else {
            dir = (dy >= 0) ? 1 : 2; /* 1:下, 2:上 */
        }

        int x1, y1, x2, y2;
        switch (dir) {
            case 0: /* → 右 */
                x1 = cx;
                y1 = cy - thick / 2;
                x2 = cx + reach;
                y2 = cy + thick / 2;
                break;
            case 1: /* ↓ 下 */
                x1 = cx - thick / 2;
                y1 = cy;
                x2 = cx + thick / 2;
                y2 = cy + reach;
                break;
            case 2: /* ↑ 上 */
                x1 = cx - thick / 2;
                y1 = cy - reach;
                x2 = cx + thick / 2;
                y2 = cy;
                break;
            default: /* ← 左 */
                x1 = cx - reach;
                y1 = cy - thick / 2;
                x2 = cx;
                y2 = cy + thick / 2;
                break;
        }

        /* 產生攻擊元素 */
                Elements *atk = New_Combat(Combat_L, x1, y1, x2, y2, TUNG_ATTACK_DAMAGE, chara->base.side);
        if (atk) sceneManager.RegisterElement(atk);

        chara->state        = ATK;
        chara->attack_timer = ATTACK_COOLDOWN_FRAMES;
    }

    /* 4) 攻擊貼圖顯示約 10 幀後恢復 */
    if (chara->attack_timer <= ATTACK_COOLDOWN_FRAMES - 10 && chara->state == ATK) {
        chara->state = STOP;
    }
}

/* --------------------------------------------------
   繪圖
   --------------------------------------------------*/
void tungtungtung_draw(Elements *self)
{
    tungtungtung *chara = static_cast<tungtungtung *>(self->entity);
    ALLEGRO_BITMAP *bmp = chara->img[chara->state];
    if (!bmp) return;

    al_draw_bitmap(bmp,
                   chara->x,
                   chara->y,
                   chara->dir ? ALLEGRO_FLIP_HORIZONTAL : 0);
}

/* --------------------------------------------------
   碰撞互動 (目前無特殊邏輯)
   --------------------------------------------------*/
void tungtungtung_interact(Elements *self) {
    /* 預留 */
}

/* --------------------------------------------------
   釋放資源
   --------------------------------------------------*/
void tungtungtung_destory(Elements *self)
{
    if (!self) return;
    tungtungtung *chara = static_cast<tungtungtung *>(self->entity);
    for (int i = 0; i < 3; ++i) {
        if (chara->img[i]) al_destroy_bitmap(chara->img[i]);
    }
    /* 釋放 hitbox 與物件本身 */
    delete chara->base.hitbox;
    free(chara);
    free(self);
}

/* --------------------------------------------------
   私有：位置同步與邊界限制
   --------------------------------------------------*/
void _tungtungtung_update_position(Elements *self, int dx, int dy)
{
    tungtungtung *chara = static_cast<tungtungtung *>(self->entity);

    chara->x += dx;
    chara->y += dy;

    /* 邊界檢查 */
    if (chara->x < 0)                       chara->x = 0;
    if (chara->y < 0)                       chara->y = 0;
    if (chara->x > DataCenter::WIDTH  - chara->width)   chara->x = DataCenter::WIDTH  - chara->width;
    if (chara->y > DataCenter::HEIGHT - chara->height)  chara->y = DataCenter::HEIGHT - chara->height;

    /* hitbox 同步 */
    Shape *hb = chara->base.hitbox;
    const double cx = hb->center_x();
    const double cy = hb->center_y();
    hb->update_center_x(cx + dx);
    hb->update_center_y(cy + dy);
}
