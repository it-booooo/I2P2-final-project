#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "bigtung.h"
#include "susu.h"              /* 提供 get_susu() 介面 */
#include "combat.h"            /* 近戰攻擊矩形 */
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h" /* _Register_elements & Combat_L */
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/* --------------------------------------------------
   追蹤／攻擊參數
   --------------------------------------------------*/
#define CHASE_SPEED            3.0f   /* 像素 / frame */
#define ARRIVE_EPSILON         60.0f  /* 抵達判定半徑 */
#define ATTACK_DISTANCE        100.0f /* 自動攻擊距離 */
#define ATTACK_COOLDOWN_FRAMES 120    /* 2 秒冷卻 (60 FPS * 2) */
#define TUNG_ATTACK_DAMAGE    1000     /* ← 在這裡調整攻擊力 */

/* --------------------------------------------------
   建構函式
   --------------------------------------------------*/
Elements *New_bigtung(int label)
{
    bigtung *entity = static_cast<bigtung *>(malloc(sizeof(bigtung)));
    Elements *pObj = New_Elements(label);
    Elements &wrapper = *pObj;
    bigtung &obj = *entity;

    /* 載入靜態貼圖 */
    const char *state_string[3] = {"stop", "move", "atk"};
    for (int i = 0; i < 3; ++i) {
        char buffer[64];
        sprintf(buffer, "assets/image/bigtung_%s.png", state_string[i]);
        obj.img[i] = al_load_bitmap(buffer);
    }

    /* 幾何資料 */
    obj.width  = al_get_bitmap_width (obj.img[0]);
    obj.height = al_get_bitmap_height(obj.img[0]);
    obj.x = 300;
    obj.y = DataCenter::HEIGHT - obj.height - 60;
    obj.base.hp   = 1000;
    obj.base.side = 1;          /* 敵方陣營 */
    

    /* 個別冷卻計時器初始化 */
    obj.attack_timer = 0;

    /* 避開出生點太靠近玩家 */
    Elements *susu_elem = get_susu();
    susu *player_ptr = NULL;
    bool need_retry = false;
    if (susu_elem) {
        Elements &susu_wrapper = *susu_elem;
        player_ptr = static_cast<susu *>(susu_wrapper.entity);
    }
    do {
        obj.x = rand() % (DataCenter::WIDTH  - obj.width);
        obj.y = rand() % (DataCenter::HEIGHT - obj.height);

        need_retry = false;
        if (player_ptr) {
            susu &player = *player_ptr;
            if (fabs(obj.x - player.x) < ARRIVE_EPSILON &&
                fabs(obj.y - player.y) < ARRIVE_EPSILON) {
                need_retry = true;
            }
        }
    } while (need_retry);

    /* 依最終座標建立 hitbox */
    obj.base.hitbox = New_Rectangle(obj.x,
                                 obj.y,
                                 obj.x + obj.width,
                                 obj.y + obj.height);

    obj.dir   = false;  /* 預設面向左 */
    obj.state = STOP;

    /* 綁定多型函式 */
    wrapper.entity = entity;
    wrapper.Draw        = bigtung_draw;
    wrapper.Update      = bigtung_update;
    wrapper.Interact    = bigtung_interact;
    wrapper.Destroy     = bigtung_destory;

    return pObj;
}

/* --------------------------------------------------
   每幀更新：固定速率追蹤 susu，近距離自動攻擊
   --------------------------------------------------*/
void bigtung_update(Elements *self)
{
    Elements &wrapper = *self;
    bigtung &chara = *static_cast<bigtung *>(wrapper.entity);

    /* 攻擊冷卻倒數 */
    if (chara.attack_timer > 0) chara.attack_timer--;

    /* 透過單例 accessor 取得 susu */
    Elements *susu_elem = get_susu();
    if (!susu_elem) return;              /* 還沒生成 susu */

    Elements &susu_wrapper = *susu_elem;
    susu &target = *static_cast<susu *>(susu_wrapper.entity);

    /* 1) 取得雙方中心點 */
    float cx = chara.x + chara.width  * 0.5f;
    float cy = chara.y + chara.height * 0.5f;
    float tx = target.x + target.width  * 0.5f;
    float ty = target.y + target.height * 0.5f;

    float dx = tx - cx;
    float dy = ty - cy;
    float dist = sqrtf(dx * dx + dy * dy);

    /* 2) 追蹤移動或停止 */
    if (dist > ARRIVE_EPSILON) {
        float vx = CHASE_SPEED * dx / dist;
        float vy = CHASE_SPEED * dy / dist;
        _bigtung_update_position(self, (int)vx, (int)vy);
        chara.dir   = (dx >= 0);
        if (chara.state != ATK) chara.state = MOVE;
    } else {
        if (chara.state != ATK) chara.state = STOP;
    }

    /* 3) 自動攻擊判定 */
    if (dist <= ATTACK_DISTANCE && chara.attack_timer == 0) {
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
        Elements *atk = New_Combat(Combat_L, x1, y1, x2, y2, TUNG_ATTACK_DAMAGE, chara.base.side);
        if (atk) _Register_elements(scene, atk);

        chara.state        = ATK;
        chara.attack_timer = ATTACK_COOLDOWN_FRAMES;
    }

    /* 4) 攻擊貼圖顯示約 10 幀後恢復 */
    if (chara.attack_timer <= ATTACK_COOLDOWN_FRAMES - 10 && chara.state == ATK) {
        chara.state = STOP;
    }
}

/* --------------------------------------------------
   繪圖
   --------------------------------------------------*/
void bigtung_draw(Elements *self)
{
    Elements &wrapper = *self;
    bigtung &chara = *static_cast<bigtung *>(wrapper.entity);
    ALLEGRO_BITMAP *bmp = chara.img[chara.state];
    if (!bmp) return;

    al_draw_bitmap(bmp,
                   chara.x,
                   chara.y,
                   chara.dir ? ALLEGRO_FLIP_HORIZONTAL : 0);
}

/* --------------------------------------------------
   碰撞互動 (目前無特殊邏輯)
   --------------------------------------------------*/
void bigtung_interact(Elements *self) {
    /* 預留 */
}

/* --------------------------------------------------
   釋放資源
   --------------------------------------------------*/
void bigtung_destory(Elements *self)
{
    if (!self) return;
    Elements &wrapper = *self;
    bigtung &chara = *static_cast<bigtung *>(wrapper.entity);
    for (int i = 0; i < 3; ++i) {
        if (chara.img[i]) al_destroy_bitmap(chara.img[i]);
    }
    /* 釋放 hitbox 與物件本身 */
    delete chara.base.hitbox;
    free(&chara);
    free(self);
}

/* --------------------------------------------------
   私有：位置同步與邊界限制
   --------------------------------------------------*/
void _bigtung_update_position(Elements *self, int dx, int dy)
{
    Elements &wrapper = *self;
    bigtung &chara = *static_cast<bigtung *>(wrapper.entity);

    chara.x += dx;
    chara.y += dy;

    /* 邊界檢查 */
    if (chara.x < 0)                       chara.x = 0;
    if (chara.y < 0)                       chara.y = 0;
    if (chara.x > DataCenter::WIDTH  - chara.width)   chara.x = DataCenter::WIDTH  - chara.width;
    if (chara.y > DataCenter::HEIGHT - chara.height)  chara.y = DataCenter::HEIGHT - chara.height;

    /* hitbox 同步 */
    Shape *hb = chara.base.hitbox;
    if (!hb) return;

    const double cx = hb->center_x();
    const double cy = hb->center_y();
    hb->update_center_x(cx + dx);
    hb->update_center_y(cy + dy);
}
