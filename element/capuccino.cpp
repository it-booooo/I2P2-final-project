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
    capuccino *p  = new capuccino{};
    Elements  *obj = New_Elements(label);

    const char *state_str[4] = { "stop", "move", "atk", "mug" };
    for (int i = 0; i < 4; ++i) {
        char buf[64];
        std::snprintf(buf, sizeof(buf),
                      "./assets/image/CapuccinoAssassino_%s.png", state_str[i]);
        p->img[i] = ImageCenter::get_instance()->get(buf);
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
    susu *player = nullptr;
    if (susu_elem) {
        player = static_cast<susu *>(susu_elem->entity);
    }

    bool overlap_player;
    do {
        p->x = std::rand() % (DataCenter::WIDTH  - p->width);
        p->y = std::rand() % (DataCenter::HEIGHT - p->height);
        overlap_player = false;
        if (player) {
            overlap_player =
                std::fabs(p->x - player->x) < ARRIVE_EPSILON &&
                std::fabs(p->y - player->y) < ARRIVE_EPSILON;
        }
    } while (overlap_player);

    p->base.hitbox = New_Rectangle(
        p->x,
        p->y,
        p->x + p->width,
        p->y + p->height
    );

    p->dir   = false;
    p->state = STOP;

    obj->entity   = p;
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
            c->state        = STOP;
            c->mug_cooldown = MUG_COOLDOWN_FRAMES;

            /* 重建 hitbox */
            if (!c->base.hitbox) {
                c->base.hitbox = New_Rectangle(
                    c->x, c->y,
                    c->x + c->width,
                    c->y + c->height
                );
            }
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
            c->base.hitbox = nullptr;
        }
        return;
    }

    /* ===== 以下為正常行為 ===== */
    if (c->attack_timer > 0) c->attack_timer--;

    Elements *susu_elem = get_susu();
    if (!susu_elem) return;
    susu *target = static_cast<susu *>(susu_elem->entity);

    float cx = c->x + c->width  * 0.5f;
    float cy = c->y + c->height * 0.5f;
    float tx = target->x + target->width  * 0.5f;
    float ty = target->y + target->height * 0.5f;
    float dx = tx - cx;
    float dy = ty - cy;
    float dist = std::sqrt(dx * dx + dy * dy);

    /* 1) 追蹤移動 */
    if (dist > ARRIVE_EPSILON) {
        float vx = CHASE_SPEED * dx / dist;
        float vy = CHASE_SPEED * dy / dist;
        _capuccino_update_position(self, static_cast<int>(vx), static_cast<int>(vy));
        c->dir = (dx >= 0);
        if (c->state != ATK) c->state = MOVE;
    } else if (c->state != ATK) {
        c->state = STOP;
    }

    /* 2) 自動攻擊 */
    if (dist <= ATTACK_DISTANCE && c->attack_timer == 0) {
        const int reach = 120;
        const int thick = 200;
        int dir = (std::fabs(dx) > std::fabs(dy))
                    ? ((dx >= 0) ? 0 : 3)
                    : ((dy >= 0) ? 1 : 2);

        int x1, y1, x2, y2;
        switch (dir) {
            case 0:
                x1 = static_cast<int>(cx);
                y1 = static_cast<int>(cy - thick / 2);
                x2 = static_cast<int>(cx + reach);
                y2 = static_cast<int>(cy + thick / 2);
                break;
            case 1:
                x1 = static_cast<int>(cx - thick / 2);
                y1 = static_cast<int>(cy);
                x2 = static_cast<int>(cx + thick / 2);
                y2 = static_cast<int>(cy + reach);
                break;
            case 2:
                x1 = static_cast<int>(cx - thick / 2);
                y1 = static_cast<int>(cy - reach);
                x2 = static_cast<int>(cx + thick / 2);
                y2 = static_cast<int>(cy);
                break;
            default:
                x1 = static_cast<int>(cx - reach);
                y1 = static_cast<int>(cy - thick / 2);
                x2 = static_cast<int>(cx);
                y2 = static_cast<int>(cy + thick / 2);
                break;
        }

        Elements *atk = New_Combat(
            Combat_L,
            x1, y1, x2, y2,
            CAPUCCINO_ATTACK_DAMAGE,
            c->base.side
        );
        if (atk) {
            sceneManager.RegisterElement(atk);
        }

        c->state        = ATK;
        c->attack_timer = ATTACK_COOLDOWN_FRAMES;
    }

    /* 3) 攻擊貼圖維持 10 禎 */
    if (c->attack_timer <= ATTACK_COOLDOWN_FRAMES - 10 &&
        c->state == ATK) {
        c->state = STOP;
    }
}

/* ---------- 繪圖 ---------- */
void capuccino_draw(Elements *self)
{
    capuccino *c = static_cast<capuccino *>(self->entity);
    ALLEGRO_BITMAP *bmp = c->img[c->state];
    if (!bmp) return;

    al_draw_bitmap(
        bmp,
        c->x,
        c->y,
        c->dir ? ALLEGRO_FLIP_HORIZONTAL : 0
    );
}

/* ---------- 互動（保留） ---------- */
void capuccino_interact(Elements *self)
{
    (void)self;
}

/* ---------- 釋放 ---------- */
void capuccino_destory(Elements *self)
{
    if (!self || !self->entity) return;

    capuccino *c = static_cast<capuccino *>(self->entity);

    // 圖片由 ImageCenter 管理，不可 al_destroy_bitmap
    if (c->base.hitbox) {
        delete c->base.hitbox;
        c->base.hitbox = nullptr;
    }

    delete c;
    self->entity = nullptr;

    // 不要 delete/free self，交給 Scene / SceneManager 管
}

/* ---------- 私有：位置同步 ---------- */
void _capuccino_update_position(Elements *self, int dx, int dy)
{
    capuccino *c = static_cast<capuccino *>(self->entity);
    c->x += dx;
    c->y += dy;

    if (c->x < 0) c->x = 0;
    if (c->y < 0) c->y = 0;
    if (c->x > DataCenter::WIDTH  - c->width)
        c->x = DataCenter::WIDTH  - c->width;
    if (c->y > DataCenter::HEIGHT - c->height)
        c->y = DataCenter::HEIGHT - c->height;

    if (c->base.hitbox) {
        Shape *hb = c->base.hitbox;
        const double cx = hb->center_x();
        const double cy = hb->center_y();
        hb->update_center_x(cx + dx);
        hb->update_center_y(cy + dy);
    }
}
