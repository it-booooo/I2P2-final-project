#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "capuccino.h"
#include "susu.h"
#include "combat.h"
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>

/* ---------- 參數 ---------- */
#define CHASE_SPEED             5.0f
#define ARRIVE_EPSILON          60.0f
#define ATTACK_DISTANCE         150.0f
#define ATTACK_COOLDOWN_FRAMES  120
#define CAPUCCINO_ATTACK_DAMAGE 50

#define MUG_DURATION_FRAMES     60   /* 1 秒 */
#define MUG_COOLDOWN_FRAMES     180  /* 3 秒 */

/* ---------- 建構 ---------- */
Elements *New_capuccino(int label)
{
    capuccino *p = static_cast<capuccino *>(malloc(sizeof(capuccino)));
    Elements  *obj = New_Elements(label);

    const char *state_str[4] = { "stop", "move", "atk", "mug" };
    for (int i = 0; i < 4; ++i) {
        char buf[64];
        sprintf(buf, "assets/image/CapuccinoAssassino_%s.png", state_str[i]);
        p->img[i] = al_load_bitmap(buf);
    }

    p->width  = al_get_bitmap_width (p->img[0]);
    p->height = al_get_bitmap_height(p->img[0]);
    p->x = 300;
    p->y = DataCenter::HEIGHT - p->height - 60;
    p->base.hp   = 50;
    p->base.side = 1;

    p->attack_timer = 0;
    p->mug_timer    = 0;
    p->mug_cooldown = MUG_COOLDOWN_FRAMES;

    /* 避免出生太近玩家 */
    Elements *susu_elem = get_susu();
    susu *player = susu_elem ? (susu *)susu_elem->entity : NULL;
    do {
        p->x = rand() % (DataCenter::WIDTH  - p->width);
        p->y = rand() % (DataCenter::HEIGHT - p->height);
    } while (player &&
             fabs(p->x - player->x) < ARRIVE_EPSILON &&
             fabs(p->y - player->y) < ARRIVE_EPSILON);

    p->base.hitbox = New_Rectangle(p->x, p->y,
                                   p->x + p->width,
                                   p->y + p->height);

    p->dir   = false;
    p->state = STOP;

    obj->entity = p;
    obj->Draw     = capuccino_draw;
    obj->Update   = capuccino_update;
    obj->Interact = capuccino_interact;
    obj->Destroy  = capuccino_destory;
    return obj;
}

/* ---------- 每禎更新 ---------- */
void capuccino_update(Elements *self)
{
    capuccino *c = static_cast<capuccino *>(self->entity);

    /* ===== Mug 狀態處理 ===== */
    if (c->mug_timer > 0) {              /* 正在 mug */
        if (--c->mug_timer == 0) {       /* 出 mug */
            c->state       = STOP;
            c->mug_cooldown = MUG_COOLDOWN_FRAMES;

            /* 重建 hitbox */
            if (!c->base.hitbox)
                c->base.hitbox = New_Rectangle(
                    c->x, c->y, c->x + c->width, c->y + c->height);
        }
        return;                          /* mug 中不做任何事 */
    }
    /* 不在 mug：倒數下一次進入 mug */
    if (--c->mug_cooldown <= 0) {
        c->state     = MUG;
        c->mug_timer = MUG_DURATION_FRAMES;

        /* 移除 hitbox 變無敵 */
        if (c->base.hitbox) {
            delete c->base.hitbox;
            c->base.hitbox = NULL;
        }
        return;                          /* 本禎結束 */
    }

    /* ===== 以下為正常行為 ===== */
    if (c->attack_timer > 0) c->attack_timer--;

    Elements *susu_elem = get_susu();
    if (!susu_elem) return;
    susu *target = (susu *)susu_elem->entity;

    float cx = c->x + c->width  * 0.5f;
    float cy = c->y + c->height * 0.5f;
    float tx = target->x + target->width  * 0.5f;
    float ty = target->y + target->height * 0.5f;
    float dx = tx - cx;
    float dy = ty - cy;
    float dist = sqrtf(dx * dx + dy * dy);

    /* 1) 追蹤移動 */
    if (dist > ARRIVE_EPSILON) {
        float vx = CHASE_SPEED * dx / dist;
        float vy = CHASE_SPEED * dy / dist;
        _capuccino_update_position(self, (int)vx, (int)vy);
        c->dir = (dx >= 0);
        if (c->state != ATK) c->state = MOVE;
    } else if (c->state != ATK) {
        c->state = STOP;
    }

    /* 2) 自動攻擊 */
    if (dist <= ATTACK_DISTANCE && c->attack_timer == 0) {
        const int reach = 120, thick = 200;
        int dir = (fabsf(dx) > fabsf(dy)) ? ((dx >= 0) ? 0 : 3)
                                          : ((dy >= 0) ? 1 : 2);
        int x1, y1, x2, y2;
        switch (dir) {
            case 0:  x1 = cx;           y1 = cy - thick / 2;
                     x2 = cx + reach;   y2 = cy + thick / 2; break;
            case 1:  x1 = cx - thick/2; y1 = cy;
                     x2 = cx + thick/2; y2 = cy + reach;     break;
            case 2:  x1 = cx - thick/2; y1 = cy - reach;
                     x2 = cx + thick/2; y2 = cy;             break;
            default: x1 = cx - reach;   y1 = cy - thick / 2;
                     x2 = cx;           y2 = cy + thick / 2;
        }
        Elements *atk = New_Combat(Combat_L, x1, y1, x2, y2,
                                    CAPUCCINO_ATTACK_DAMAGE, c->base.side);
        if (atk) sceneManager.RegisterElement(atk);

        c->state        = ATK;
        c->attack_timer = ATTACK_COOLDOWN_FRAMES;
    }

    /* 3) 攻擊貼圖維持 10 禎 */
    if (c->attack_timer <= ATTACK_COOLDOWN_FRAMES - 10 &&
        c->state == ATK)
        c->state = STOP;
}

/* ---------- 繪圖 ---------- */
void capuccino_draw(Elements *self)
{
    capuccino *c = static_cast<capuccino *>(self->entity);
    ALLEGRO_BITMAP *bmp = c->img[c->state];
    if (bmp)
        al_draw_bitmap(bmp, c->x, c->y,
                       c->dir ? ALLEGRO_FLIP_HORIZONTAL : 0);
}

/* ---------- 互動（保留） ---------- */
void capuccino_interact(Elements *self) { }

/* ---------- 釋放 ---------- */
void capuccino_destory(Elements *self)
{
    if (!self) return;
    capuccino *c = static_cast<capuccino *>(self->entity);
    for (int i = 0; i < 4; ++i)
        if (c->img[i]) al_destroy_bitmap(c->img[i]);
    if (c->base.hitbox) delete c->base.hitbox;
    free(c);
    free(self);
}

/* ---------- 私有：位置同步 ---------- */
void _capuccino_update_position(Elements *self, int dx, int dy)
{
    capuccino *c = static_cast<capuccino *>(self->entity);
    c->x += dx; c->y += dy;

    if (c->x < 0)                      c->x = 0;
    if (c->y < 0)                      c->y = 0;
    if (c->x > DataCenter::WIDTH  - c->width)      c->x = DataCenter::WIDTH  - c->width;
    if (c->y > DataCenter::HEIGHT - c->height)     c->y = DataCenter::HEIGHT - c->height;

    if (c->base.hitbox) {
        Shape *hb = c->base.hitbox;
        const double cx = hb->center_x();
        const double cy = hb->center_y();
        hb->update_center_x(cx + dx);
        hb->update_center_y(cy + dy);
    }
}
