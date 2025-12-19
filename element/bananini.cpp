/* Bananini ― 會射 banana.png */
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "bananini.h"
#include "susu.h"
#include "atk.h"
#include "damageable.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "../scene/gamescene.h"

/* 參數同上 */
#define CHASE_SPEED     2.0f
#define ARRIVE_EPSILON 400.0f
#define COOLDOWN_FRAMES 180
#define BULLET_DAMAGE   40
#define BULLET_SPEED    3.0f
#define SHOT_RANGE      500.0f
static void _banana_update_position(Elements *self, int dx, int dy);

/* 建構 */
Elements *New_bananini(int label)
{
    //  1. 改用 new，往 C++ 風格靠
    auto *pD = new bananini{};      // 所有成員先歸零
    Elements *pE = New_Elements(label);

    //  2. 貼圖改由 ImageCenter 管理
    const char *state_s[3] = {"stop", "move", "atk"};
    for (int i = 0; i < 3; ++i) {
        char buf[64];
        std::snprintf(buf, sizeof(buf),
                      "./assets/image/ChimpanziniBananini_%s.png", state_s[i]);
        pD->img[i] = ImageCenter::get_instance()->get(buf);
    }

    pD->width  = al_get_bitmap_width (pD->img[0]);
    pD->height = al_get_bitmap_height(pD->img[0]);

    Elements   *player_ele = get_current_player();
    Damageable *player     = nullptr;
    double px = 0.0, py = 0.0;

    if (player_ele && player_ele->entity) {
        player = reinterpret_cast<Damageable *>(player_ele->entity);
        if (player->hitbox) {
            px = player->hitbox->center_x();
            py = player->hitbox->center_y();
        }
    }

    while (true) {
        pD->x = std::rand() % (DataCenter::WIDTH  - pD->width);
        pD->y = std::rand() % (DataCenter::HEIGHT - pD->height);

        if (!player || !player->hitbox) break;

        float ex = pD->x + pD->width  * 0.5f;
        float ey = pD->y + pD->height * 0.5f;

        float dx = ex - (float)px;
        float dy = ey - (float)py;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist >= ARRIVE_EPSILON) break;
    }

    pD->base.hp   = 60;
    pD->base.side = 1;
    pD->base.hitbox = New_Rectangle(
        pD->x,
        pD->y,
        pD->x + pD->width,
        pD->y + pD->height
    );

    pD->dir      = false;
    pD->state    = STOP;
    pD->cooldown = 0;

    pE->entity   = pD;
    pE->Draw     = bananini_draw;
    pE->Update   = bananini_update;
    pE->Interact = bananini_interact;
    pE->Destroy  = bananini_destory;
    return pE;
}

/* Update：射 banana.png */
void bananini_update(Elements *self)
{
    auto *ch = static_cast<bananini *>(self->entity);
    if (ch->cooldown > 0) ch->cooldown--;

    Elements *player_ele = get_current_player();
    if (!player_ele || !player_ele->entity) return;

    Damageable *pl = reinterpret_cast<Damageable *>(player_ele->entity);
    if (!pl->hitbox) return;

    int cx = ch->x + ch->width  / 2;
    int cy = ch->y + ch->height / 2;
    int tx = (int)pl->hitbox->center_x();
    int ty = (int)pl->hitbox->center_y();

    int dx = tx - cx;
    int dy = ty - cy;
    float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));

    if (dist > SHOT_RANGE) {
        float vx = CHASE_SPEED * dx / dist;
        float vy = CHASE_SPEED * dy / dist;
        _banana_update_position(self, static_cast<int>(vx), static_cast<int>(vy));
        ch->dir   = (dx >= 0);
        ch->state = MOVE;
    } else {
        ch->state = STOP;
    }

    if (ch->state == STOP && ch->cooldown == 0) {
        if (dist < 1.0f) dist = 1.0f;
        float fx = BULLET_SPEED * dx / dist;
        float fy = BULLET_SPEED * dy / dist;
        int vx = static_cast<int>(std::round(fx));
        int vy = static_cast<int>(std::round(fy));
        if (vx == 0) vx = (dx > 0) ? 1 : -1;
        if (vy == 0) vy = (dy > 0) ? 1 : -1;

        Elements *proj = New_Atk(
            Atk_L,
            cx - 20,
            cy - 20,
            static_cast<float>(vx),
            static_cast<float>(vy),
            BULLET_DAMAGE,
            1
        );
        if (proj) {
            //  這裡假設 Atk_set_image 已經改成使用 ImageCenter
            Atk_set_image(proj, "./assets/image/Banana.png");
            sceneManager.RegisterElement(proj);
        }
        ch->cooldown = COOLDOWN_FRAMES;
    }
}

void bananini_draw(Elements *self)
{
    auto *ch = static_cast<bananini *>(self->entity);
    ALLEGRO_BITMAP *bmp = ch->img[ch->state];
    if (!bmp) return;
    al_draw_bitmap(
        bmp,
        ch->x,
        ch->y,
        ch->dir ? ALLEGRO_FLIP_HORIZONTAL : 0
    );
}

void bananini_interact(Elements *self) {}

/* Destroy：只釋放自己 new 出來的東西，不動 bitmap、也不刪 self */
void bananini_destory(Elements *self)
{
    if (!self || !self->entity) return;

    auto *ch = static_cast<bananini *>(self->entity);

    //  不再 al_destroy_bitmap，因為 img[] 是 ImageCenter 管的共用資源
    // for (int i = 0; i < 3; ++i)
    //     if (ch->img[i]) al_destroy_bitmap(ch->img[i]);

    delete ch->base.hitbox;
    ch->base.hitbox = nullptr;

    delete ch;              //  對應 new bananini{}
    self->entity = nullptr;

    //  不要 free(self)，Elements* 由 Scene / SceneManager 管
}

static void _banana_update_position(Elements *self, int dx, int dy)
{
    auto *ch = static_cast<bananini *>(self->entity);
    ch->x += dx;
    ch->y += dy;

    if (ch->x < 0) ch->x = 0;
    if (ch->y < 0) ch->y = 0;
    if (ch->x > DataCenter::WIDTH  - ch->width)
        ch->x = DataCenter::WIDTH  - ch->width;
    if (ch->y > DataCenter::HEIGHT - ch->height)
        ch->y = DataCenter::HEIGHT - ch->height;

    Shape *hb = ch->base.hitbox;
    if (!hb) return;

    const double cx = hb->center_x();
    const double cy = hb->center_y();
    hb->update_center_x(cx + dx);
    hb->update_center_y(cy + dy);
}
