/* ----------------- tralala.c (waterwave置中版) ----------------- */
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "tralala.h"
#include "susu.h"
#include "combat.h"
#include "earthquake.h"
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h"
#include "../shapes/Rectangle.h"
#include "../shapes/Circle.h"
#include "../global.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

/* ---------------- 參數 ---------------- */
#define CHASE_SPEED             4.0f
#define ARRIVE_EPSILON         60.0f
#define ATTACK_DISTANCE       150.0f
#define ATTACK_COOLDOWN_FRAMES 120
#define TRALALA_ATTACK_DAMAGE   50

#define EARTHQUAKE_RANGE       600.0f  /* 觸發距離 */
#define EARTHQUAKE_COOLDOWN    120     /* 2 秒 */
#define EARTHQUAKE_DAMAGE       15

/* ---------------- 建構 ---------------- */
Elements *New_tralala(int label)
{
    tralala *pDerivedObj = malloc(sizeof(tralala));
    Elements *pObj = New_Elements(label);

    const char *state_string[3] = {"stop", "move", "atk"};
    for (int i = 0; i < 3; ++i) {
        char buf[64];
        sprintf(buf, "assets/image/tralala_%s.png", state_string[i]);
        pDerivedObj->img[i] = al_load_bitmap(buf);
    }

    pDerivedObj->width  = al_get_bitmap_width (pDerivedObj->img[0]);
    pDerivedObj->height = al_get_bitmap_height(pDerivedObj->img[0]);
    pDerivedObj->x = 300;
    pDerivedObj->y = HEIGHT - pDerivedObj->height - 60;
    pDerivedObj->base.hp   = 1000;
    pDerivedObj->base.side = 1;

    pDerivedObj->attack_timer = 0;
    pDerivedObj->quake_timer  = 0;

    /* 避開出生點太靠近玩家 */
    Elements *susu_elem = get_susu();
    susu *player = susu_elem ? (susu *)susu_elem->pDerivedObj : NULL;
    do {
        pDerivedObj->x = rand() % (WIDTH  - pDerivedObj->width);
        pDerivedObj->y = rand() % (HEIGHT - pDerivedObj->height);
    } while (player &&
             fabs(pDerivedObj->x - player->x) < ARRIVE_EPSILON &&
             fabs(pDerivedObj->y - player->y) < ARRIVE_EPSILON);

    pDerivedObj->base.hitbox = New_Rectangle(pDerivedObj->x,
                                             pDerivedObj->y,
                                             pDerivedObj->x + pDerivedObj->width,
                                             pDerivedObj->y + pDerivedObj->height);

    pDerivedObj->dir = false;
    pDerivedObj->state = STOP;

    /* 綁定多型函式 */
    pObj->pDerivedObj = pDerivedObj;
    pObj->Draw        = tralala_draw;
    pObj->Update      = tralala_update;
    pObj->Interact    = tralala_interact;
    pObj->Destroy     = tralala_destory;
    return pObj;
}

/* ---------------- 更新 ---------------- */
void tralala_update(Elements *self)
{
    tralala *chara = self->pDerivedObj;
    if (chara->attack_timer > 0) chara->attack_timer--;
    if (chara->quake_timer  > 0) chara->quake_timer--;

    Elements *susu_elem = get_susu();
    if (!susu_elem) return;

    susu *target = (susu *)susu_elem->pDerivedObj;

    /* 中心點 */
    float cx = chara->x + chara->width  * 0.5f;
    float cy = chara->y + chara->height * 0.5f;
    float tx = target->x + target->width  * 0.5f;
    float ty = target->y + target->height * 0.5f;

    float dx = tx - cx;
    float dy = ty - cy;
    float dist = sqrtf(dx*dx + dy*dy);

    /* 1. 追蹤移動 */
    if (dist > ARRIVE_EPSILON) {
        float vx = CHASE_SPEED * dx / dist;
        float vy = CHASE_SPEED * dy / dist;
        _tralala_update_position(self, (int)vx, (int)vy);
        chara->dir = (dx >= 0);
        if (chara->state != ATK) chara->state = MOVE;
    } else if (chara->state != ATK) {
        chara->state = STOP;
    }

    /* 2. 近戰攻擊 */
    if (dist <= ATTACK_DISTANCE && chara->attack_timer == 0) {
        const int reach = 120;
        const int thick = 200;
        int dir = (fabsf(dx) > fabsf(dy)) ? ((dx >= 0) ? 0 : 3)
                                          : ((dy >= 0) ? 1 : 2);
        int x1,y1,x2,y2;
        switch (dir) {
            case 0: x1=cx; y1=cy-thick/2; x2=cx+reach;   y2=cy+thick/2; break;
            case 1: x1=cx-thick/2; y1=cy; x2=cx+thick/2; y2=cy+reach;    break;
            case 2: x1=cx-thick/2; y1=cy-reach; x2=cx+thick/2; y2=cy;   break;
            default:x1=cx-reach; y1=cy-thick/2; x2=cx; y2=cy+thick/2;   break;
        }
        Elements *atk = New_Combat(Combat_L, x1, y1, x2, y2,
                                   TRALALA_ATTACK_DAMAGE, chara->base.side);
        if (atk) _Register_elements(scene, atk);
        chara->state = ATK;
        chara->attack_timer = ATTACK_COOLDOWN_FRAMES;
    }
    if (chara->attack_timer <= ATTACK_COOLDOWN_FRAMES - 10 && chara->state == ATK)
        chara->state = STOP;

    /* 3. 距離判定 + 冷卻判定 → 產生 Earthquake, 換成 waterwave 並置中 */
    if (dist <= EARTHQUAKE_RANGE && chara->quake_timer == 0) {
        float centerX = chara->x + chara->width  * 0.5f;
        float centerY = chara->y + chara->height * 0.5f;

        /* 先用中心點作為暫時的 x,y 建立物件 */
        Elements *quake = New_Earthquake(Earthquake_L,
                                         (int)centerX,
                                         (int)centerY,
                                         EARTHQUAKE_DAMAGE,
                                         chara->base.side);
        if (quake) {
            Earthquake *eq = (Earthquake *)quake->pDerivedObj;
            /* 換貼圖 */
            al_destroy_bitmap(eq->img);
            eq->img = al_load_bitmap("assets/image/waterwave.png");
            /* 重新取得寬高 */
            eq->width  = al_get_bitmap_width(eq->img);
            eq->height = al_get_bitmap_height(eq->img);
            /* 置中到 tralala 中心 */
            eq->x = centerX - eq->width  / 2;
            eq->y = centerY - eq->height / 2;
            /* 重建 hitbox，使中心即在 tralala 中心 */
            free(eq->hitbox);
            eq->hitbox = New_Circle(centerX,
                                    centerY,
                                    fminf(eq->width,eq->height) / 2);
            _Register_elements(scene, quake);
        }
        chara->quake_timer = EARTHQUAKE_COOLDOWN;
    }
}

/* ---------------- 繪圖 ---------------- */
void tralala_draw(Elements *self)
{
    tralala *chara = self->pDerivedObj;
    ALLEGRO_BITMAP *bmp = chara->img[chara->state];
    if (!bmp) return;
    al_draw_bitmap(bmp,
                   chara->x,
                   chara->y,
                   chara->dir ? ALLEGRO_FLIP_HORIZONTAL : 0);
}

/* ---------------- 目前無額外碰撞邏輯 ---------------- */
void tralala_interact(Elements *self)
{
    /* 預留 */
}

/* ---------------- 釋放資源 ---------------- */
void tralala_destory(Elements *self)
{
    if (!self) return;
    tralala *chara = self->pDerivedObj;
    for (int i = 0; i < 3; ++i)
        if (chara->img[i]) al_destroy_bitmap(chara->img[i]);
    free(chara->base.hitbox);
    free(chara);
    free(self);
}

/* ---------------- 私有：同步位置 & 邊界 ---------------- */
void _tralala_update_position(Elements *self, int dx, int dy)
{
    tralala *chara = self->pDerivedObj;
    chara->x += dx;
    chara->y += dy;

    /* 邊界限制 */
    if (chara->x < 0)                          chara->x = 0;
    if (chara->y < 0)                          chara->y = 0;
    if (chara->x > WIDTH  - chara->width)      chara->x = WIDTH  - chara->width;
    if (chara->y > HEIGHT - chara->height)     chara->y = HEIGHT - chara->height;

    /* hitbox 同步 */
    Shape *hb = chara->base.hitbox;
    hb->update_center_x(hb, dx);
    hb->update_center_y(hb, dy);
}
