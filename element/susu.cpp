#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <cstdio>
#include <cmath>

#include "susu.h"
#include "hpbar.h"
#include "projectile.h"
#include "atk.h"
#include "combat.h"
#include "earthquake.h"
#include "bloodman.h"          // ★ 新增：需要 Bloodman 型別 + get_bloodman()
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include "../algif5/algif.h"
#include "../scene/gamescene.h"
#include <allegro5/allegro_native_dialog.h>
#include <cstdio>
#include <cmath>
#include "../data/DataCenter.h"

#define M_PI 3.14159265358979323846


/*
   [susu function]
*/

int gControlledCharacter = 1;   // ★ 一開始控制角色1

// ★ 目前操作中的玩家（給血條、怪物用）
Elements *get_current_player(void)
{
    if (gControlledCharacter == 1)
        return get_susu();
    else if (gControlledCharacter == 2)
        return get_bloodman();
    return nullptr;
}

static Elements *singleton_susu = NULL; // CHANGED: added singleton pointer to expose susu to other modules

Elements *get_susu(void) // CHANGED: accessor to retrieve the singleton pointer
{
    return singleton_susu;
}

// ★ 切換角色時，同步兩個角色的位置＋hitbox
void SyncCharactersOnSwitch(int newChar)
{
    Elements *s_ele = get_susu();
    Elements *b_ele = get_bloodman();

    if (!s_ele || !b_ele || !s_ele->entity || !b_ele->entity)
    {
        // 其中一個還沒生成，就只改控制權
        gControlledCharacter = newChar;
        return;
    }

    susu     *s  = static_cast<susu    *>(s_ele->entity);
    Bloodman *bm = static_cast<Bloodman*>(b_ele->entity);

    if (gControlledCharacter == 1 && newChar == 2)
    {
        // 從角色1 → 角色2：讓 bloodman 繼承 susu 的位置
        bm->x = s->x;
        bm->y = s->y;

        if (s->base.hitbox && bm->base.hitbox)
        {
            double cx = s->base.hitbox->center_x();
            double cy = s->base.hitbox->center_y();
            bm->base.hitbox->update_center_x(cx);
            bm->base.hitbox->update_center_y(cy);
        }
    }
    else if (gControlledCharacter == 2 && newChar == 1)
    {
        // 從角色2 → 角色1：讓 susu 繼承 bloodman 的位置（你現在想要的行為）
        s->x = bm->x;
        s->y = bm->y;

        if (s->base.hitbox && bm->base.hitbox)
        {
            double cx = bm->base.hitbox->center_x();
            double cy = bm->base.hitbox->center_y();
            s->base.hitbox->update_center_x(cx);
            s->base.hitbox->update_center_y(cy);
        }
    }

    gControlledCharacter = newChar;
}

Elements *New_susu(int label)
{
    //  改用 new，而不是 malloc
    susu *entity = new susu{};
    Elements *pObj = New_Elements(label);

    // load susu images (GIF)
    char state_string[5][10] = {"stop_2", "move_2", "attack_2","combat_2","earth_2"};
    for (int i = 0; i < 5; i++)
    {
        char buffer[50];
        std::sprintf(buffer, "assets/image/chara_%s.gif", state_string[i]);
        entity->gif_status[i] = algif_load_animation(buffer);
    }
    //entity->img = al_load_bitmap("assets/image/susu_1.png");
    // load effective sound
    ALLEGRO_SAMPLE *sample = al_load_sample("assets/sound/atk_sound.wav");
    entity->atk_Sound = al_create_sample_instance(sample);
    al_set_sample_instance_playmode(entity->atk_Sound, ALLEGRO_PLAYMODE_ONCE);
    al_attach_sample_instance_to_mixer(entity->atk_Sound, al_get_default_mixer());

    // initial the geometric information of susu
    entity->width  = entity->gif_status[0]->width;
    entity->height = entity->gif_status[0]->height;
    entity->x = 300;
    entity->y = DataCenter::HEIGHT - entity->height - 60;
    entity->base.hitbox = New_Rectangle(entity->x+200,
                                        entity->y+50,
                                        entity->x + entity->width-200,
                                        entity->y + entity->height-50);
    entity->base.hp      = 10000;
    entity->base.full_hp = 10000;
    entity->base.side    = 0;

    entity->dir    = false; // true: face to right, false: face to left
    entity->damage = 80;

    // initial the animation component
    entity->state = STOP;
    entity->new_proj = false;
    entity->e_timer = 0;
    entity->q_timer = 0;
    entity->anime      = 0;
    entity->anime_time = 0;

    pObj->entity = entity;
    // setting derived object function
    pObj->Draw = susu_draw;
    pObj->Update = susu_update;
    pObj->Interact = susu_interact;
    pObj->Destroy  = susu_destroy;

    singleton_susu = pObj;
    return pObj;
}

void susu_update(Elements *self)
{
    susu *chara = static_cast<susu *>(self->entity);

    if (chara->e_timer > 0) chara->e_timer--;
    if (chara->q_timer > 0) chara->q_timer--;

    int move_dis = 10;
    static bool space = 0;
    int space_co = 15;

    ALLEGRO_MOUSE_STATE state;
    al_get_mouse_state(&state);
    DataCenter *DC = DataCenter::get_instance();

    ALLEGRO_MOUSE_STATE mstate;
    al_get_mouse_state(&mstate);

    // ★ 角色切換：只在「按下瞬間」觸發，並同步位置
    bool press1 = DC->key_state[ALLEGRO_KEY_1] && !DC->prev_key_state[ALLEGRO_KEY_1];
    bool press2 = DC->key_state[ALLEGRO_KEY_2] && !DC->prev_key_state[ALLEGRO_KEY_2];

    if (press1 && gControlledCharacter != 1)
    {
        SyncCharactersOnSwitch(1);   // 切回角色1（susu），繼承角色2的位置
    }
    else if (press2 && gControlledCharacter != 2)
    {
        SyncCharactersOnSwitch(2);   // 切到角色2（bloodman），繼承角色1的位置
    }

    // ★ 不在操控角色時，不吃輸入
    if (gControlledCharacter != 1)
    {
        chara->state = STOP;   // 或者乾脆什麼都不改
        return;
    }

    if (DC->key_state[ALLEGRO_KEY_SPACE] == 0)
        space = 0;

    // ============================================================
    // 新增動畫計時系統（不再依賴 GIF 的 done）
    if (chara->anime_time > 0) {
        chara->anime++;
        if (chara->anime >= chara->anime_time) {
            chara->state      = STOP;
            chara->anime      = 0;
            chara->anime_time = 0;
            chara->new_proj   = false;
        }
    }
    // ============================================================

    // ===================== STOP =====================
    if (chara->state == STOP)
    {
        if (state.buttons & 1)
        {
            chara->state = COMBAT;
            chara->anime = 0;
            chara->anime_time = 20;
            chara->new_proj = false;
        }
        else if (DC->key_state[ALLEGRO_KEY_Q])
        {
            if (chara->q_timer <= 0)
            {
                chara->state = ATK;
                chara->anime = 0;
                chara->anime_time = 30;
                chara->new_proj = false;
            }
        }
        else if (DC->key_state[ALLEGRO_KEY_E])
        {
            if (chara->e_timer <= 0)
            {
                chara->state = EARTHQUAKE;
                chara->anime = 0;
                chara->anime_time = 45;
                chara->new_proj = false;
            }
        }
        else if (DC->key_state[ALLEGRO_KEY_SPACE] && space==0)
        {
            if(chara->dir==0)       _susu_update_position(self, -1*move_dis*space_co, 0);
            else if(chara->dir==1)  _susu_update_position(self, move_dis*space_co, 0);
            else if(chara->dir==2)  _susu_update_position(self, 0, -1*move_dis*space_co);
            else if(chara->dir==3)  _susu_update_position(self, 0, move_dis*space_co);
            space = 1;
            chara->state = MOVE;
        }
        else if (DC->key_state[ALLEGRO_KEY_A])
        {
            chara->dir = 0;
            chara->state = MOVE;
        }
        else if (DC->key_state[ALLEGRO_KEY_D])
        {
            chara->dir = 1;
            chara->state = MOVE;
        }
        else if (DC->key_state[ALLEGRO_KEY_W])
        {
            chara->dir = 2;
            chara->state = MOVE;
        }
        else if (DC->key_state[ALLEGRO_KEY_S])
        {
            chara->dir = 3;
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
        if (state.buttons & 1)
        {
            chara->state = COMBAT;
            chara->anime = 0;
            chara->anime_time = 20;
            chara->new_proj = false;
        }
        else if (DC->key_state[ALLEGRO_KEY_Q])
        {
            if (chara->q_timer <= 0)
            {
                chara->state = ATK;
                chara->anime = 0;
                chara->anime_time = 30;
                chara->new_proj = false;
            }
        }
        else if (DC->key_state[ALLEGRO_KEY_E])
        {
            if (chara->e_timer <= 0)
            {
                chara->state = EARTHQUAKE;
                chara->anime = 0;
                chara->anime_time = 45;
                chara->new_proj = false;
            }
        }
        else if (DC->key_state[ALLEGRO_KEY_SPACE] && space==0)
        {
            if(chara->dir==0)       _susu_update_position(self, -1*move_dis*space_co, 0);
            else if(chara->dir==1)  _susu_update_position(self, move_dis*space_co, 0);
            else if(chara->dir==2)  _susu_update_position(self, 0, -1*move_dis*space_co);
            else if(chara->dir==3)  _susu_update_position(self, 0, move_dis*space_co);
            space = 1;
        }
        else if (DC->key_state[ALLEGRO_KEY_A])
        {
            chara->dir = 0;
            _susu_update_position(self, -1*move_dis, 0);
        }
        else if (DC->key_state[ALLEGRO_KEY_D])
        {
            chara->dir = 1;
            _susu_update_position(self, move_dis, 0);
        }
        else if (DC->key_state[ALLEGRO_KEY_W])
        {
            chara->dir = 2;
            _susu_update_position(self, 0, -1*move_dis);
        }
        else if (DC->key_state[ALLEGRO_KEY_S])
        {
            chara->dir = 3;
            _susu_update_position(self, 0, move_dis);
        }
        else
        {
            chara->state = STOP;
        }
    }

    // ===================== ATK =====================
    else if (chara->state == ATK)
    {
        if (chara->gif_status[ATK]->display_index == 2 &&
            chara->new_proj == false &&
            chara->q_timer <= 0)
        {
            chara->q_timer = 60;

            float dx = DC->mouse.x - (chara->x + chara->width*0.5f);
            float dy = DC->mouse.y - (chara->y + chara->height*0.5f);
            float len = std::sqrt(dx * dx + dy * dy);
            float speed = 25.0f;
            float vx = speed * dx / len;
            float vy = speed * dy / len;

            Elements *pro = New_Atk(
                Atk_L,
                static_cast<int>(chara->x + chara->width*0.5f - 20.0f),
                static_cast<int>(chara->y + chara->height*0.5f - 70.0f),
                vx, vy,
                chara->damage * 2,
                0
            );
            if (pro) sceneManager.RegisterElement(pro);

            chara->new_proj = true;
        }
    }

    // ===================== COMBAT =====================
    else if (chara->state == COMBAT)
    {
        if (chara->gif_status[COMBAT]->display_index == 3 &&
            chara->new_proj == false)
        {
            float cx = chara->x + chara->width*0.5f;
            float cy = chara->y + chara->height*0.5f;

            float dx = DC->mouse.x - cx;
            float dy = DC->mouse.y - cy;

            const int reach = 200;
            const int thick = 300;

            float angle = std::atan2(dy, dx);
            int dir;
            if (angle > -M_PI/4 && angle <=  M_PI/4)        dir = 0;
            else if (angle >  M_PI/4 && angle <= 3*M_PI/4)  dir = 1;
            else if (angle > -3*M_PI/4 && angle <=-M_PI/4)  dir = 2;
            else                                            dir = 3;

            int x1,y1,x2,y2;
            switch (dir) {
                case 0:
                    x1 = static_cast<int>(cx);
                    y1 = static_cast<int>(cy - thick/2);
                    x2 = static_cast<int>(cx + reach);
                    y2 = static_cast<int>(cy + thick/2);
                    break;
                case 1:
                    x1 = static_cast<int>(cx - thick/2);
                    y1 = static_cast<int>(cy);
                    x2 = static_cast<int>(cx + thick/2);
                    y2 = static_cast<int>(cy + reach);
                    break;
                case 2:
                    x1 = static_cast<int>(cx - thick/2);
                    y1 = static_cast<int>(cy - reach);
                    x2 = static_cast<int>(cx + thick/2);
                    y2 = static_cast<int>(cy);
                    break;
                default:
                    x1 = static_cast<int>(cx - reach);
                    y1 = static_cast<int>(cy - thick/2);
                    x2 = static_cast<int>(cx);
                    y2 = static_cast<int>(cy + thick/2);
                    break;
            }

            Elements *pro = New_Combat(
                Combat_L,
                x1, y1, x2, y2,
                chara->damage,
                0
            );
            if (pro) sceneManager.RegisterElement(pro);

            chara->new_proj = true;
        }
    }

    // ===================== EARTHQUAKE =====================
    else if (chara->state == EARTHQUAKE)
    {
        if (chara->gif_status[EARTHQUAKE]->display_index == 4 &&
            chara->new_proj == false &&
            chara->e_timer <= 0)
        {
            chara->e_timer = 60;

            Elements *pro = New_Earthquake(
                Earthquake_L,
                static_cast<int>(chara->x + chara->width*0.5f - 192.0f),
                static_cast<int>(chara->y + chara->height*0.5f - 100.0f),
                chara->damage,
                0
            );
            if (pro) sceneManager.RegisterElement(pro);

            chara->new_proj = true;
        }
    }
}

void susu_draw(Elements *self)
{
    // ★ 非操作中的角色不畫出來（數值仍保留）
    if (gControlledCharacter != 1)
        return;

    // with the state, draw corresponding image
    susu *chara = ((susu *)(self->entity));
    DataCenter *DC = DataCenter::get_instance();

    ALLEGRO_BITMAP *frame =
        algif_get_bitmap(chara->gif_status[chara->state], al_get_time());

    if (frame)
    {
        al_draw_bitmap(
            frame,
            chara->x,
            chara->y,
            ((DC->mouse.x - (chara->x + chara->width*0.5f)) > 0)
                ? ALLEGRO_FLIP_HORIZONTAL : 0
        );
    }

    //  改用 SoundCenter 播放攻擊音效
    if (chara->state == ATK &&
        chara->gif_status[chara->state] &&
        chara->gif_status[chara->state]->display_index == 2)
    {
        SoundCenter::get_instance()->play("assets/sound/atk_sound.wav",
                                          ALLEGRO_PLAYMODE_ONCE);
    }
}

void susu_destroy(Elements *self)
{
    if (!self || !self->entity) return;

    susu *Obj = static_cast<susu *>(self->entity);

    //  不再 al_destroy_sample_instance(Obj->atk_Sound);
    //    音效改由 SoundCenter 管理

    // 銷毀全部 5 個 GIF
    for (int i = 0; i < 5; i++) {
        if (Obj->gif_status[i]) {
            algif_destroy_animation(Obj->gif_status[i]);
            Obj->gif_status[i] = nullptr;
        }
    }

    if (Obj->base.hitbox) {
        delete Obj->base.hitbox;
        Obj->base.hitbox = nullptr;
    }

    delete Obj;
    self->entity = nullptr;

    singleton_susu = nullptr;  // 單例清掉

    //  不要 free(self)，交給 Scene / SceneManager 管
}

void _susu_update_position(Elements *self, int dx, int dy)
{
    susu *chara = static_cast<susu *>(self->entity);

    if (chara->x + chara->width*0.5 == 0   && dx < 0) dx = 0;
    if (chara->x + chara->width*0.5 ==1800 && dx > 0) dx = 0;
    if (chara->y + chara->height - 200 == 0 && dy < 0) dy = 0;
    if (chara->y + chara->height == 1400    && dy > 0) dy = 0;

    chara->x += dx;
    chara->y += dy;

    Shape *hitbox = chara->base.hitbox;
    if (!hitbox) return;

    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);
}

void susu_interact(Elements *self) {}
