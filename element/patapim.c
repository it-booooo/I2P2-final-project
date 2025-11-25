#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include "patapim.h"
#include "susu.h"
#include "combat.h"
#include "earthquake.h" // ★ 引入地震技能
#include "../scene/sceneManager.h"
#include "../scene/gamescene.h"
#include "../shapes/Rectangle.h"
#include "../global.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define CHASE_SPEED            3.0f
#define ARRIVE_EPSILON         60.0f
#define ATTACK_DISTANCE        150.0f
#define ATTACK_COOLDOWN_FRAMES 120
#define EARTHQUAKE_COOLDOWN    60   // ★ 每60幀產生一次地震攻擊
#define patapim_ATTACK_DAMAGE  50
#define EARTHQUAKE_DAMAGE      10

Elements *New_patapim(int label) {
    patapim *pDerivedObj = malloc(sizeof(patapim));
    Elements *pObj = New_Elements(label);

    const char *state_string[3] = {"stop", "move", "atk"};
    for (int i = 0; i < 3; ++i) {
        char buffer[64];
        sprintf(buffer, "assets/image/patapim_%s.png", state_string[i]);
        pDerivedObj->img[i] = al_load_bitmap(buffer);
    }

    pDerivedObj->width  = al_get_bitmap_width (pDerivedObj->img[0]);
    pDerivedObj->height = al_get_bitmap_height(pDerivedObj->img[0]);
    pDerivedObj->x = 300;
    pDerivedObj->y = HEIGHT - pDerivedObj->height - 60;
    pDerivedObj->base.hp   = 50;
    pDerivedObj->base.side = 1;

    pDerivedObj->attack_timer = 0;
    pDerivedObj->quake_timer  = 0; // ★ 初始為0

    Elements *susu_elem = get_susu();
    susu *player = NULL;
    if (susu_elem) player = (susu *)susu_elem->pDerivedObj;
    do {
        pDerivedObj->x = rand() % (WIDTH  - pDerivedObj->width);
        pDerivedObj->y = rand() % (HEIGHT - pDerivedObj->height);
    } while (player && fabs(pDerivedObj->x - player->x) < ARRIVE_EPSILON &&
                       fabs(pDerivedObj->y - player->y) < ARRIVE_EPSILON);

    pDerivedObj->base.hitbox = New_Rectangle(pDerivedObj->x,
                                             pDerivedObj->y,
                                             pDerivedObj->x + pDerivedObj->width,
                                             pDerivedObj->y + pDerivedObj->height);

    pDerivedObj->dir   = false;
    pDerivedObj->state = STOP;

    pObj->pDerivedObj = pDerivedObj;
    pObj->Draw        = patapim_draw;
    pObj->Update      = patapim_update;
    pObj->Interact    = patapim_interact;
    pObj->Destroy     = patapim_destory;

    return pObj;
}

void patapim_update(Elements *self) {
    patapim *chara = self->pDerivedObj;
    if (chara->attack_timer > 0) chara->attack_timer--;
    if (chara->quake_timer > 0) chara->quake_timer--; // ★ quake timer

    Elements *susu_elem = get_susu();
    if (!susu_elem) return;

    susu *target = (susu *)susu_elem->pDerivedObj;

    float cx = chara->x + chara->width  * 0.5f;
    float cy = chara->y + chara->height * 0.5f;
    float tx = target->x + target->width  * 0.5f;
    float ty = target->y + target->height * 0.5f;

    float dx = tx - cx;
    float dy = ty - cy;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist > ARRIVE_EPSILON) {
        float vx = CHASE_SPEED * dx / dist;
        float vy = CHASE_SPEED * dy / dist;
        _patapim_update_position(self, (int)vx, (int)vy);
        chara->dir   = (dx >= 0);
        if (chara->state != ATK) chara->state = MOVE;
    } else {
        if (chara->state != ATK) chara->state = STOP;
    }

    if (dist <= ATTACK_DISTANCE && chara->attack_timer == 0) {
        const int reach = 120;
        const int thick = 200;

        int dir = (fabsf(dx) > fabsf(dy)) ? ((dx >= 0) ? 0 : 3) : ((dy >= 0) ? 1 : 2);
        int x1, y1, x2, y2;
        switch (dir) {
            case 0: x1 = cx; y1 = cy - thick/2; x2 = cx + reach; y2 = cy + thick/2; break;
            case 1: x1 = cx - thick/2; y1 = cy; x2 = cx + thick/2; y2 = cy + reach; break;
            case 2: x1 = cx - thick/2; y1 = cy - reach; x2 = cx + thick/2; y2 = cy; break;
            default:x1 = cx - reach; y1 = cy - thick/2; x2 = cx; y2 = cy + thick/2; break;
        }

        Elements *atk = New_Combat(Combat_L, x1, y1, x2, y2, patapim_ATTACK_DAMAGE, chara->base.side);
        if (atk) _Register_elements(scene, atk);

        chara->state        = ATK;
        chara->attack_timer = ATTACK_COOLDOWN_FRAMES;
    }

    if (chara->attack_timer <= ATTACK_COOLDOWN_FRAMES - 10 && chara->state == ATK) {
        chara->state = STOP;
    }

    // ★ 自動釋放 earthquake 技能
    if (chara->quake_timer <= 0) {
        Elements *quake = New_Earthquake(Earthquake_L,
            chara->x + chara->width / 2,
            chara->y + chara->height / 2,
            EARTHQUAKE_DAMAGE,
            chara->base.side);
        if (quake){

             Earthquake *eq = (Earthquake *)quake->pDerivedObj;
            /* 換貼圖 */
            al_destroy_bitmap(eq->img);
            eq->img = al_load_bitmap("assets/image/boom.png");
            _Register_elements(scene, quake);
        } 
        chara->quake_timer = EARTHQUAKE_COOLDOWN;
    }
}

void patapim_draw(Elements *self) {
    patapim *chara = self->pDerivedObj;
    ALLEGRO_BITMAP *bmp = chara->img[chara->state];
    if (!bmp) return;
    al_draw_bitmap(bmp, chara->x, chara->y, chara->dir ? ALLEGRO_FLIP_HORIZONTAL : 0);
}

void patapim_interact(Elements *self) {}

void patapim_destory(Elements *self) {
    if (!self) return;
    patapim *chara = self->pDerivedObj;
    for (int i = 0; i < 3; ++i) {
        if (chara->img[i]) al_destroy_bitmap(chara->img[i]);
    }
    free(chara->base.hitbox);
    free(chara);
    free(self);
}

void _patapim_update_position(Elements *self, int dx, int dy) {
    patapim *chara = self->pDerivedObj;
    chara->x += dx;
    chara->y += dy;
    if (chara->x < 0) chara->x = 0;
    if (chara->y < 0) chara->y = 0;
    if (chara->x > WIDTH - chara->width) chara->x = WIDTH - chara->width;
    if (chara->y > HEIGHT - chara->height) chara->y = HEIGHT - chara->height;
    Shape *hb = chara->base.hitbox;
    hb->update_center_x(hb, dx);
    hb->update_center_y(hb, dy);
}
