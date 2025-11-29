#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "crocodilo.h"
#include "susu.h"
#include "atk.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../global.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../scene/gamescene.h"

/* ---------- Crocodilo 參數（與原版相同） ---------- */
#define CHASE_SPEED      3.0f
#define ARRIVE_EPSILON 400.0f
#define COOLDOWN_FRAMES 180
#define BULLET_DAMAGE    400
#define BULLET_SPEED     7.0f
#define SHOT_RANGE     1000.0f

/* ---------- 子彈追蹤用 ---------- */
#define HOMING_FRAMES 120           /* 2 秒（60 FPS × 2） */

typedef struct _HomingEntry {
    Elements             *bullet;   /* 子彈本尊（Elements*） */
    int                   frames;   /* 還剩多少 frame 要追蹤 */
    struct _HomingEntry  *next;
} HomingEntry;

static HomingEntry *_homing_list = NULL;

/* forward declarations */
static void _croco_update_position(Elements *self, int dx, int dy);
static void _register_homing_bullet(Elements *proj);
static void _croco_bullet_update(Elements *self);

/* ---------- Crocodilo 建構 ---------- */
Elements *New_crocodilo(int label)
{
    crocodilo *pD = malloc(sizeof(crocodilo));
    Elements  *pE = New_Elements(label);

    const char *state_s[3] = { "stop", "move", "atk" };
    for (int i = 0; i < 3; ++i) {
        char buf[64];
        sprintf(buf, "assets/image/crocodilo_%s.png", state_s[i]);
        pD->img[i] = al_load_bitmap(buf);
    }
    pD->width  = al_get_bitmap_width (pD->img[0]);
    pD->height = al_get_bitmap_height(pD->img[0]);

    Elements *plE = get_susu();
    susu     *pl  = plE ? (susu *)plE->pDerivedObj : NULL;
    do {
        pD->x = rand() % (WIDTH  - pD->width );
        pD->y = rand() % (HEIGHT - pD->height);
    } while (pl && fabs(pD->x - pl->x) < ARRIVE_EPSILON &&
                   fabs(pD->y - pl->y) < ARRIVE_EPSILON);

    pD->base.hp     = 600;
    pD->base.side   = 1;
    pD->base.hitbox = New_Rectangle(pD->x,           pD->y,
                                    pD->x + pD->width,
                                    pD->y + pD->height);

    pD->dir      = false;
    pD->state    = STOP;
    pD->cooldown = 0;

    pE->pDerivedObj = pD;
    pE->Draw        = crocodilo_draw;
    pE->Update      = crocodilo_update;
    pE->Interact    = crocodilo_interact;
    pE->Destroy     = crocodilo_destory;
    return pE;
}

/* ---------- 一般行為 / 射擊 ---------- */
void crocodilo_update(Elements *self)
{
    crocodilo *ch = self->pDerivedObj;
    if (ch->cooldown > 0) ch->cooldown--;

    Elements *plE = get_susu();          if (!plE) return;
    susu     *pl  = plE->pDerivedObj;

    int cx = ch->x + ch->width  / 2,
        cy = ch->y + ch->height / 2;
    int tx = pl->x + pl->width  / 2,
        ty = pl->y + pl->height / 2;
    int dx = tx - cx,
        dy = ty - cy;
    float dist = sqrtf((float)dx * dx + (float)dy * dy);

    /* 追人移動 ---------------------------------------------------- */
    if (dist > SHOT_RANGE) {
        float vx = CHASE_SPEED * dx / dist,
              vy = CHASE_SPEED * dy / dist;
        _croco_update_position(self, (int)vx, (int)vy);
        ch->dir   = (dx >= 0);
        ch->state = MOVE;
    } else {
        ch->state = STOP;
    }

    /* 射擊 -------------------------------------------------------- */
    if (ch->state == STOP && ch->cooldown == 0) {
        if (dist < 1.0f) dist = 1.0f;               /* 避免除以 0 */

        float fx = BULLET_SPEED * dx / dist,
              fy = BULLET_SPEED * dy / dist;
        int vx = (int)roundf(fx),
            vy = (int)roundf(fy);
        if (vx == 0) vx = (dx > 0) ? 1 : -1;
        if (vy == 0) vy = (dy > 0) ? 1 : -1;

        Elements *proj = New_Atk(Atk_L, cx - 20, cy - 20,
                                 (float)vx, (float)vy,
                                 BULLET_DAMAGE, 1);
        if (proj) {
            Atk_set_image(proj, "assets/image/bullet.png");
            _Register_elements(scene, proj);

            /* 關鍵：登記追蹤行為，換 Update 指標 */
            _register_homing_bullet(proj);
        }
        ch->cooldown = COOLDOWN_FRAMES;
    }
}

/* ---------- 登記追蹤用子彈 ---------- */
static void _register_homing_bullet(Elements *proj)
{
    HomingEntry *node = malloc(sizeof(HomingEntry));
    node->bullet = proj;
    node->frames = HOMING_FRAMES;
    node->next   = _homing_list;
    _homing_list = node;

    proj->Update = _croco_bullet_update;   /* 換掉原本的 Atk_update */
}

/* ---------- 子彈 Update：先追蹤、後直線 ---------- */
static void _croco_bullet_update(Elements *self)
{
    /* 找到對應的 list 節點 */
    HomingEntry *prev = NULL, *cur = _homing_list;
    while (cur && cur->bullet != self) {
        prev = cur;  cur = cur->next;
    }

    /* 萬一沒找到，就退回原本行為 */
    if (!cur) {
        Atk_update(self);
        return;
    }

    /* 還在追蹤期：每幀重新鎖定玩家 */
    if (cur->frames > 0) {
        cur->frames--;

        Elements *plE = get_susu();
        if (plE) {
            susu *pl   = plE->pDerivedObj;
            Atk  *atk  = self->pDerivedObj;

            int cx = atk->x + atk->width  / 2,
                cy = atk->y + atk->height / 2;
            int tx = pl->x + pl->width  / 2,
                ty = pl->y + pl->height / 2;
            int dx = tx - cx,
                dy = ty - cy;
            float dist = sqrtf((float)dx * dx + (float)dy * dy);
            if (dist < 1.0f) dist = 1.0f;

            /* 維持既定速度大小（或直接使用常數 BULLET_SPEED） */
            float speed = BULLET_SPEED;
            atk->vx = speed * dx / dist;
            atk->vy = speed * dy / dist;
        }
    }

    /* 讓 Atk 做位移、碰撞、刪除檢查 */
    Atk_update(self);

    /* 若子彈被標記刪除 → 從 list 移除並 free 節點 */
    if (self->dele) {
        if (prev) prev->next = cur->next;
        else      _homing_list = cur->next;
        free(cur);
    }
}

/* ---------- 其餘函式維持原樣 ---------- */
void crocodilo_draw(Elements *self)
{
    crocodilo *ch = self->pDerivedObj;
    ALLEGRO_BITMAP *bmp = ch->img[ch->state];
    if (!bmp) return;
    al_draw_bitmap(bmp, ch->x, ch->y,
                   ch->dir ? ALLEGRO_FLIP_HORIZONTAL : 0);
}

void crocodilo_interact(Elements *self) { /* 目前 Crocodilo 不做互動 */ }

void crocodilo_destory(Elements *self)
{
    crocodilo *ch = self->pDerivedObj;
    for (int i = 0; i < 3; ++i)
        if (ch->img[i]) al_destroy_bitmap(ch->img[i]);
    free(ch->base.hitbox);
    free(ch);
    free(self);
}

static void _croco_update_position(Elements *self, int dx, int dy)
{
    crocodilo *ch = self->pDerivedObj;
    ch->x += dx;  ch->y += dy;

    /* 場邊碰壁 */
    if (ch->x < 0)                       ch->x = 0;
    if (ch->y < 0)                       ch->y = 0;
    if (ch->x > WIDTH  - ch->width )     ch->x = WIDTH  - ch->width ;
    if (ch->y > HEIGHT - ch->height)     ch->y = HEIGHT - ch->height;

    Shape *hb = ch->base.hitbox;
    hb->update_center_x(hb, dx);
    hb->update_center_y(hb, dy);
}
