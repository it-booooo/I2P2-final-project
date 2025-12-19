#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro.h>

#include "bloodman.h"
#include "atk.h"
#include "combat.h"
#include "earthquake.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include "../algif5/algif.h"
#include "../scene/gamescene.h"
#include "../data/DataCenter.h"

#include <cstdio>
#include <cmath>
#include <iostream>

#define M_PI 3.14159265358979323846

// 這裡不定義 gControlledCharacter，使用 susu.cpp 那個定義的 extern
extern int gControlledCharacter;

// ★ 從 susu.cpp 來的同步函式（只要宣告就好，定義在 susu.cpp 裡）
void SyncCharactersOnSwitch(int newChar);

/*
   [Bloodman singleton]
*/
static Elements *singleton_bloodman = NULL;

Elements *get_bloodman(void)
{
    return singleton_bloodman;
}

Elements *New_Bloodman(int label)   // ★ 名稱改成 New_Bloodman，對應 header / GameScene 呼叫
{
    std::printf("[New_Bloodman] called, singleton_bloodman=%p\n", singleton_bloodman);
    
    Bloodman *entity = new Bloodman{};
    Elements *pObj = New_Elements(label);

    // const char state_string[5][10] = {"stop_2", "move_2", "attack_2", "combat_2", "earth_2"};
    // for (int i = 0; i < 5; i++)
    // {
    //     char buffer[50];
    //     sprintf(buffer, "assets/image/chara_%s.gif", state_string[i]);
    //     entity->gif_status[i] = algif_load_animation(buffer);
    // }
    entity->img=ImageCenter::get_instance()->get("assets/image/bloodman.png");
    if(!entity->img)
        std::fprintf(stderr, "[New_Bloodman] Failed to load bloodman image\n");
    ALLEGRO_SAMPLE *sample = al_load_sample("assets/sound/atk_sound.wav");
    entity->atk_Sound = al_create_sample_instance(sample);
    al_set_sample_instance_playmode(entity->atk_Sound, ALLEGRO_PLAYMODE_ONCE);
    al_attach_sample_instance_to_mixer(entity->atk_Sound, al_get_default_mixer());

    entity->width  = al_get_bitmap_width(entity->img);
    entity->height = al_get_bitmap_height(entity->img);
    entity->x = 600; // 跟 susu 稍微分開
    entity->y = DataCenter::HEIGHT - entity->height - 60;

    entity->base.hitbox = New_Rectangle(entity->x + 200,
                                        entity->y + 50,
                                        entity->x + entity->width - 200,
                                        entity->y + entity->height - 50);
    entity->base.hp      = 10000;
    entity->base.full_hp = 10000;
    entity->base.side    = 0; // 仍然視為玩家陣營

    entity->dir    = 0;
    entity->damage = 80;

    entity->state      = STOP;
    entity->new_proj   = false;
    entity->e_timer    = 0;
    entity->q_timer    = 0;
    entity->anime      = 0;
    entity->anime_time = 0;

    entity->lifesteal_active = false;
    entity->lifesteal_timer  = 0;

    pObj->entity   = entity;
    pObj->Draw     = bloodman_draw;
    pObj->Update   = bloodman_update;
    pObj->Interact = bloodman_interact;
    pObj->Destroy  = bloodman_destroy;

    singleton_bloodman = pObj;

    std::printf("[New_Bloodman] new entity=%p, hp=%d\n", entity, entity->base.hp);
    return pObj;
}

void bloodman_update(Elements *self)
{
    Bloodman *chara = (Bloodman *)(self->entity);

    if (chara->e_timer > 0) chara->e_timer--;
    if (chara->q_timer > 0) chara->q_timer--;

    // Q 吸血 buff 計時（假設 60 fps，10 秒 ≈ 600 frame）
    if (chara->lifesteal_timer > 0)
    {
        chara->lifesteal_timer--;
        if (chara->lifesteal_timer == 0)
            chara->lifesteal_active = false;
    }

    // 動畫計時
    if (chara->anime_time > 0)
    {
        chara->anime++;
        if (chara->anime >= chara->anime_time)
        {
            chara->state      = STOP;
            chara->anime      = 0;
            chara->anime_time = 0;
            chara->new_proj   = false;
        }
    }

    int move_dis = 10;
    static bool space = false;
    int space_co = 15;

    ALLEGRO_MOUSE_STATE mstate;
    al_get_mouse_state(&mstate);
    DataCenter *DC = DataCenter::get_instance();

    // ★ 角色切換：只在「按下瞬間」觸發，並同步位置
    bool press1 = DC->key_state[ALLEGRO_KEY_1] && !DC->prev_key_state[ALLEGRO_KEY_1];
    bool press2 = DC->key_state[ALLEGRO_KEY_2] && !DC->prev_key_state[ALLEGRO_KEY_2];

    if (press1 && gControlledCharacter != 1)
        SyncCharactersOnSwitch(1);
    else if (press2 && gControlledCharacter != 2)
        SyncCharactersOnSwitch(2);

    // 沒被操控時，不吃輸入
    if (gControlledCharacter != 2)
    {
        chara->state = STOP;
        return;
    }

    if (DC->key_state[ALLEGRO_KEY_SPACE] == 0)
        space = false;

    // ===================== STOP =====================
    if (chara->state == STOP)
    {
        if (mstate.buttons & 1)
        {
            chara->state      = COMBAT;
            chara->anime      = 0;
            chara->anime_time = 20;
            chara->new_proj   = false;
        }
        else if (DC->key_state[ALLEGRO_KEY_Q])
        {
            // Q：扣除目前 50% 血量 + 開 10 秒吸血 buff（沒有投擲物）
            if (chara->q_timer <= 0 && chara->base.hp > 1)
            {
                int cost = chara->base.hp / 2;
                if (cost <= 0) cost = 1;
                chara->base.hp -= cost;
                if (chara->base.hp <= 0) chara->base.hp = 1;

                chara->lifesteal_active = true;
                chara->lifesteal_timer  = 600;

                chara->q_timer = 60; // Q 冷卻

                chara->state      = ATK;
                chara->anime      = 0;
                chara->anime_time = 30;
                chara->new_proj   = false;
            }
        }
        else if (DC->key_state[ALLEGRO_KEY_E])
        {
            // E：改成丟投擲物（原本 Q 的效果），取消地震
            if (chara->e_timer <= 0)
            {
                chara->state      = EARTHQUAKE;
                chara->anime      = 0;
                chara->anime_time = 30;
                chara->new_proj   = false;
            }
        }
        else if (DC->key_state[ALLEGRO_KEY_SPACE] && !space)
        {
            if      (chara->dir == 0) _bloodman_update_position(self, -move_dis * space_co, 0);
            else if (chara->dir == 1) _bloodman_update_position(self,  move_dis * space_co, 0);
            else if (chara->dir == 2) _bloodman_update_position(self, 0, -move_dis * space_co);
            else if (chara->dir == 3) _bloodman_update_position(self, 0,  move_dis * space_co);
            space = true;
            chara->state = MOVE;
        }
        else if (DC->key_state[ALLEGRO_KEY_A])
        {
            chara->dir   = 0;
            chara->state = MOVE;
        }
        else if (DC->key_state[ALLEGRO_KEY_D])
        {
            chara->dir   = 1;
            chara->state = MOVE;
        }
        else if (DC->key_state[ALLEGRO_KEY_W])
        {
            chara->dir   = 2;
            chara->state = MOVE;
        }
        else if (DC->key_state[ALLEGRO_KEY_S])
        {
            chara->dir   = 3;
            chara->state = MOVE;
        }
        else
        {
            chara->state = STOP;
        }
    }
    // ===================== MOVE =====================
    else if (chara->state == MOVE)
    {
        if (mstate.buttons & 1)
        {
            chara->state      = COMBAT;
            chara->anime      = 0;
            chara->anime_time = 20;
            chara->new_proj   = false;
        }
        else if (DC->key_state[ALLEGRO_KEY_Q])
        {
            if (chara->q_timer <= 0 && chara->base.hp > 1)
            {
                int cost = chara->base.hp / 2;
                if (cost <= 0) cost = 1;
                chara->base.hp -= cost;
                if (chara->base.hp <= 0) chara->base.hp = 1;

                chara->lifesteal_active = true;
                chara->lifesteal_timer  = 600;

                chara->q_timer = 60;

                chara->state      = ATK;
                chara->anime      = 0;
                chara->anime_time = 30;
                chara->new_proj   = false;
            }
        }
        else if (DC->key_state[ALLEGRO_KEY_E])
        {
            if (chara->e_timer <= 0)
            {
                chara->state      = EARTHQUAKE;
                chara->anime      = 0;
                chara->anime_time = 30;
                chara->new_proj   = false;
            }
        }
        else if (DC->key_state[ALLEGRO_KEY_SPACE] && !space)
        {
            if      (chara->dir == 0) _bloodman_update_position(self, -move_dis * space_co, 0);
            else if (chara->dir == 1) _bloodman_update_position(self,  move_dis * space_co, 0);
            else if (chara->dir == 2) _bloodman_update_position(self, 0, -move_dis * space_co);
            else if (chara->dir == 3) _bloodman_update_position(self, 0,  move_dis * space_co);
            space = true;
        }
        else if (DC->key_state[ALLEGRO_KEY_A])
        {
            chara->dir = 0;
            _bloodman_update_position(self, -move_dis, 0);
        }
        else if (DC->key_state[ALLEGRO_KEY_D])
        {
            chara->dir = 1;
            _bloodman_update_position(self, move_dis, 0);
        }
        else if (DC->key_state[ALLEGRO_KEY_W])
        {
            chara->dir = 2;
            _bloodman_update_position(self, 0, -move_dis);
        }
        else if (DC->key_state[ALLEGRO_KEY_S])
        {
            chara->dir = 3;
            _bloodman_update_position(self, 0, move_dis);
        }
        else
        {
            chara->state = STOP;
        }
    }
    // ===================== ATK（這裡只有動畫，不再丟投擲物） =====================
    else if (chara->state == ATK)
    {
        // Q 技能現在只有自傷 + 開 buff，真正的吸血效果在 DealDamageIfPossible 裡判斷
    }
    // ===================== COMBAT（近戰） =====================
    else if (chara->state == COMBAT)
    {
        if (!chara->new_proj)
        {
            float cx = chara->x + chara->width * 0.5f;
            float cy = chara->y + chara->height * 0.5f;

            float dx = DC->mouse.x - cx;
            float dy = DC->mouse.y - cy;

            const int reach = 200;
            const int thick = 300;

            float angle = atan2f(dy, dx);
            int dir;
            if      (angle > -M_PI/4 && angle <=  M_PI/4)      dir = 0;
            else if (angle >  M_PI/4 && angle <= 3*M_PI/4)     dir = 1;
            else if (angle > -3*M_PI/4 && angle <= -M_PI/4)    dir = 2;
            else                                               dir = 3;

            int x1,y1,x2,y2;
            switch (dir) {
                case 0:
                    x1 = cx;          y1 = cy - thick/2;
                    x2 = cx + reach;  y2 = cy + thick/2;
                    break;
                case 1:
                    x1 = cx - thick/2;  y1 = cy;
                    x2 = cx + thick/2;  y2 = cy + reach;
                    break;
                case 2:
                    x1 = cx - thick/2;  y1 = cy - reach;
                    x2 = cx + thick/2;  y2 = cy;
                    break;
                default:
                case 3:
                    x1 = cx - reach;    y1 = cy - thick/2;
                    x2 = cx;            y2 = cy + thick/2;
                    break;
            }

            Elements *pro = New_Combat(Combat_L, x1, y1, x2, y2, chara->damage, 0);
            if (pro) sceneManager.RegisterElement(pro);

            chara->new_proj = true;
        }
    }
    // ===================== EARTHQUAKE（現在變成 E 投擲物） =====================
    else if (chara->state == EARTHQUAKE)
    {
        if (!chara->new_proj &&
            chara->e_timer <= 0)
        {
            chara->e_timer = 60;

            float cx = chara->x + chara->width * 0.5f;
            float cy = chara->y + chara->height * 0.5f;

            float dx = DC->mouse.x - cx;
            float dy = DC->mouse.y - cy;
            float len = sqrtf(dx * dx + dy * dy);
            if (len == 0) len = 1;

            float speed = 25.0f;
            float vx = speed * dx / len;
            float vy = speed * dy / len;

            Elements *pro = New_Atk(Atk_L,
                                     cx - 20.0f,
                                     cy - 70.0f,
                                     vx, vy,
                                     chara->damage * 2,
                                     0);
            if (pro) sceneManager.RegisterElement(pro);

            chara->new_proj = true;
        }
    }
}

void bloodman_draw(Elements *self)
{
    // ★ 非操作中的角色不畫出來（數值仍保留）
    if (gControlledCharacter != 2)
        return;

    Bloodman *chara = (Bloodman *)(self->entity);
    DataCenter *DC = DataCenter::get_instance();

    // ALLEGRO_BITMAP *frame = algif_get_bitmap(chara->gif_status[chara->state], al_get_time());
    // if (frame)
    // {
    //     float cx = chara->x + chara->width * 0.5f;
    //     int flags = (DC->mouse.x - cx > 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    //     al_draw_bitmap(frame, chara->x, chara->y, flags);
    // }
    const float scale = 0.25f;

    const float sw = al_get_bitmap_width(chara->img);
    const float sh = al_get_bitmap_height(chara->img);
    const float dw = sw * scale;
    const float dh = sh * scale;

    int flags = ((DC->mouse.x - (chara->x + dw * 0.5f)) > 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;

    al_draw_scaled_bitmap(
        chara->img,
        0, 0, sw, sh,          // 來源：整張圖
        chara->x+150, chara->y,     // 目的地左上角
        dw, dh,                 // 縮放後寬高
        flags
    );

    if (chara->atk_Sound && chara->state == ATK)
    {
        al_play_sample_instance(chara->atk_Sound);
    }
}

void bloodman_destroy(Elements *self)
{
    Bloodman *Obj = (Bloodman *)(self->entity);
    std::printf("[bloodman_destroy] called, Obj=%p, hp=%d\n", Obj, Obj->base.hp);

    al_destroy_sample_instance(Obj->atk_Sound);

    // for (int i = 0; i < 5; i++)
    // {
    //     if (Obj->gif_status[i])
    //         algif_destroy_animation(Obj->gif_status[i]);
    // }

    delete Obj->base.hitbox;
    Obj->base.hitbox = nullptr;
    delete Obj;
    self->entity = nullptr;
    singleton_bloodman = nullptr;

    std::printf("[bloodman_destroy] done, singleton_bloodman reset to nullptr\n");
    
}

void _bloodman_update_position(Elements *self, int dx, int dy)
{
    Bloodman *chara = (Bloodman *)(self->entity);

    if (chara->x + chara->width * 0.5 <= 0 && dx < 0)       dx = 0;
    if (chara->x + chara->width * 0.5 >= 1800 && dx > 0)    dx = 0;
    if (chara->y + chara->height - 200 <= 0 && dy < 0)      dy = 0;
    if (chara->y + chara->height >= 1400 && dy > 0)         dy = 0;

    chara->x += dx;
    chara->y += dy;

    Shape *hitbox = chara->base.hitbox;
    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);
}

void bloodman_interact(Elements *self)
{
    // 目前沒有額外互動
}
