#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "patapim.h"
#include "susu.h"
#include "combat.h"
#include "earthquake.h"
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include "../data/ImageCenter.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>

#define CHASE_SPEED            3.0f
#define ARRIVE_EPSILON         60.0f
#define ATTACK_DISTANCE        150.0f
#define ATTACK_COOLDOWN_FRAMES 120
#define EARTHQUAKE_COOLDOWN    60
#define patapim_ATTACK_DAMAGE  50
#define EARTHQUAKE_DAMAGE      10

Elements *New_patapim(int label)
{
    patapim *entity = new patapim{};
    Elements *pObj  = New_Elements(label);
    Elements &wrapper = *pObj;
    patapim &obj = *entity;

    const char *state_string[3] = {"stop", "move", "atk"};
    for (int i = 0; i < 3; ++i) {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer),
                      "./assets/image/patapim_%s.png", state_string[i]);
        obj.img[i] = ImageCenter::get_instance()->get(buffer);
    }

    obj.width  = al_get_bitmap_width (obj.img[0]);
    obj.height = al_get_bitmap_height(obj.img[0]);
    obj.x = 300;
    obj.y = DataCenter::HEIGHT - obj.height - 60;
    obj.base.hp   = 50;
    obj.base.side = 1;

    obj.attack_timer = 0;
    obj.quake_timer  = 0;

    Elements *susu_elem = get_susu();
    susu *player = nullptr;
    if (susu_elem) {
        Elements &susu_wrapper = *susu_elem;
        player = static_cast<susu *>(susu_wrapper.entity);
    }

    bool overlap_player;
    do {
        obj.x = std::rand() % (DataCenter::WIDTH  - obj.width);
        obj.y = std::rand() % (DataCenter::HEIGHT - obj.height);
        overlap_player = false;
        if (player) {
            susu &player_ref = *player;
            overlap_player =
                std::fabs(obj.x - player_ref.x) < ARRIVE_EPSILON &&
                std::fabs(obj.y - player_ref.y) < ARRIVE_EPSILON;
        }
    } while (overlap_player);

    obj.base.hitbox = New_Rectangle(
        obj.x,
        obj.y,
        obj.x + obj.width,
        obj.y + obj.height
    );

    obj.dir   = false;
    obj.state = STOP;

    wrapper.entity   = entity;
    wrapper.Draw     = patapim_draw;
    wrapper.Update   = patapim_update;
    wrapper.Interact = patapim_interact;
    wrapper.Destroy  = patapim_destory;

    return pObj;
}

void patapim_update(Elements *self)
{
    Elements &wrapper = *self;
    patapim &chara = *static_cast<patapim *>(wrapper.entity);
    if (chara.attack_timer > 0) chara.attack_timer--;
    if (chara.quake_timer  > 0) chara.quake_timer--;

    Elements *susu_elem = get_susu();
    if (!susu_elem) return;

    Elements &susu_wrapper = *susu_elem;
    susu &target = *static_cast<susu *>(susu_wrapper.entity);

    float cx = chara.x + chara.width  * 0.5f;
    float cy = chara.y + chara.height * 0.5f;
    float tx = target.x + target.width  * 0.5f;
    float ty = target.y + target.height * 0.5f;

    float dx = tx - cx;
    float dy = ty - cy;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > ARRIVE_EPSILON) {
        float vx = CHASE_SPEED * dx / dist;
        float vy = CHASE_SPEED * dy / dist;
        _patapim_update_position(self, static_cast<int>(vx), static_cast<int>(vy));
        chara.dir   = (dx >= 0);
        if (chara.state != ATK) chara.state = MOVE;
    } else {
        if (chara.state != ATK) chara.state = STOP;
    }

    if (dist <= ATTACK_DISTANCE && chara.attack_timer == 0) {
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
            patapim_ATTACK_DAMAGE,
            chara.base.side
        );
        if (atk) {
            sceneManager.RegisterElement(atk);
        }

        chara.state        = ATK;
        chara.attack_timer = ATTACK_COOLDOWN_FRAMES;
    }

    if (chara.attack_timer <= ATTACK_COOLDOWN_FRAMES - 10 &&
        chara.state == ATK) {
        chara.state = STOP;
    }

    if (chara.quake_timer <= 0) {
        Elements *quake = New_Earthquake(
            Earthquake_L,
            chara.x + chara.width / 2,
            chara.y + chara.height / 2,
            EARTHQUAKE_DAMAGE,
            chara.base.side
        );
        if (quake) {
            Elements &quake_wrapper = *quake;
            Earthquake &eq = *static_cast<Earthquake *>(quake_wrapper.entity);

            eq.img = ImageCenter::get_instance()->get("./assets/image/boom.png");
            sceneManager.RegisterElement(quake);
        }
        chara.quake_timer = EARTHQUAKE_COOLDOWN;
    }
}

void patapim_draw(Elements *self)
{
    Elements &wrapper = *self;
    patapim &chara = *static_cast<patapim *>(wrapper.entity);
    ALLEGRO_BITMAP *bmp = chara.img[chara.state];
    if (!bmp) return;
    al_draw_bitmap(
        bmp,
        chara.x,
        chara.y,
        chara.dir ? ALLEGRO_FLIP_HORIZONTAL : 0
    );
}

void patapim_interact(Elements *self)
{
    (void)self;
}

void patapim_destory(Elements *self)
{
    if (!self || !self->entity) return;

    patapim *chara = static_cast<patapim *>(self->entity);

    delete chara->base.hitbox;
    chara->base.hitbox = nullptr;

    delete chara;
    self->entity = nullptr;

    // 不要刪 self，交給 Scene / SceneManager 管理
}

void _patapim_update_position(Elements *self, int dx, int dy)
{
    Elements &wrapper = *self;
    patapim &chara = *static_cast<patapim *>(wrapper.entity);

    chara.x += dx;
    chara.y += dy;
    if (chara.x < 0) chara.x = 0;
    if (chara.y < 0) chara.y = 0;
    if (chara.x > DataCenter::WIDTH  - chara.width)
        chara.x = DataCenter::WIDTH  - chara.width;
    if (chara.y > DataCenter::HEIGHT - chara.height)
        chara.y = DataCenter::HEIGHT - chara.height;

    Shape *hb = chara.base.hitbox;
    if (!hb) return;

    const double cx = hb->center_x();
    const double cy = hb->center_y();
    hb->update_center_x(cx + dx);
    hb->update_center_y(cy + dy);
}
