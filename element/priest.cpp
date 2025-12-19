#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <cstdio>
#include <cmath>

#include "tungtungtung.h"      // ★ E技能拉怪需要 tungtungtung 型別與 tungtungtung_update()
#include "priest.h"
#include "susu.h"
#include "hpbar.h"
#include "projectile.h"
#include "atk.h"
#include "combat.h"
#include "earthquake.h"
#include "bloodman.h"          // ★ 需要 Bloodman 型別 + get_bloodman()
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include "../algif5/algif.h"
#include "../scene/gamescene.h"
#include "../data/DataCenter.h"

#define M_PI 3.14159265358979323846

/*
   [priest function]
*/

// ★ priest 單例
static Elements *singleton_priest = NULL;

Elements *get_priest(void)
{
    return singleton_priest;
}

// ===================== E技能：把所有怪物拉到前面 =====================
// ★ 目前先支援 tungtungtung；
static void _priest_pull_all_monsters_to_front(priest *chara)
{
    if (!chara) return;

    const int pull_dis = 250;   // 拉到前面多遠（可調）
    int target_x = chara->x;
    int target_y = chara->y;

    if (chara->dir == 0)       target_x -= pull_dis;  // left
    else if (chara->dir == 1)  target_x += pull_dis;  // right
    else if (chara->dir == 2)  target_y -= pull_dis;  // up
    else if (chara->dir == 3)  target_y += pull_dis;  // down

    ElementVec vec = sceneManager.GetAllElements();
    for (int i = 0; i < vec.len; i++)
    {
        Elements *e = vec.arr[i];
        if (!e || !e->entity) continue;

        // ★ 只拉 tungtungtung，避免誤 cast 其他元素造成 crash
        if (e->Update != tungtungtung_update) continue;

        tungtungtung *enemy = static_cast<tungtungtung *>(e->entity);

        // 只拉怪（side == 1），並且必須有 hitbox 才動
        if (enemy->base.side != 1) continue;
        if (!enemy->base.hitbox) continue;

        // 邊界保護（使用 DataCenter 尺寸 + 怪物自身寬高）
        int tx = target_x;
        int ty = target_y;

        if (tx < 0) tx = 0;
        if (ty < 0) ty = 0;
        if (tx > DataCenter::WIDTH  - enemy->width)  tx = DataCenter::WIDTH  - enemy->width;
        if (ty > DataCenter::HEIGHT - enemy->height) ty = DataCenter::HEIGHT - enemy->height;

        int dx = tx - enemy->x;
        int dy = ty - enemy->y;

        enemy->x = tx;
        enemy->y = ty;

        Shape *hitbox = enemy->base.hitbox;
        const double cx = hitbox->center_x();
        const double cy = hitbox->center_y();
        hitbox->update_center_x(cx + dx);
        hitbox->update_center_y(cy + dy);
    }
}
// =====================================================================

Elements *New_priest(int label)
{
    priest *entity = new priest{};
    Elements *pObj = New_Elements(label);

    // load priest images (GIF)  // ★ 路徑不動
    char state_string[5][10] = {"stop_2", "move_2", "attack_2","combat_2","earth_2"};
    for (int i = 0; i < 5; i++)
    {
        char buffer[50];
        std::sprintf(buffer, "assets/image/chara_%s.gif", state_string[i]);
        entity->gif_status[i] = algif_load_animation(buffer);
    }

    // load effective sound  // ★ 路徑不動
    ALLEGRO_SAMPLE *sample = al_load_sample("assets/sound/atk_sound.wav");
    entity->atk_Sound = al_create_sample_instance(sample);
    al_set_sample_instance_playmode(entity->atk_Sound, ALLEGRO_PLAYMODE_ONCE);
    al_attach_sample_instance_to_mixer(entity->atk_Sound, al_get_default_mixer());

    // initial the geometric information of priest
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
    pObj->Draw = priest_draw;
    pObj->Update = priest_update;
    pObj->Interact = priest_interact;
    pObj->Destroy  = priest_destroy;

    singleton_priest = pObj;
    return pObj;
}

void priest_update(Elements *self)
{
    priest *chara = static_cast<priest *>(self->entity);

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

    // ★ 不在操控角色時，不吃輸入
    if (gControlledCharacter != 3)
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

        // ★ Q技能：無任何技能（注意：不留空的 else-if，避免卡住移動）
        // （所以這裡不寫 Q 分支）

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
            if(chara->dir==0)       _priest_update_position(self, -1*move_dis*space_co, 0);
            else if(chara->dir==1)  _priest_update_position(self, move_dis*space_co, 0);
            else if(chara->dir==2)  _priest_update_position(self, 0, -1*move_dis*space_co);
            else if(chara->dir==3)  _priest_update_position(self, 0, move_dis*space_co);
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

        // ★ Q技能：無任何技能（不留空 else-if）
        // （所以這裡不寫 Q 分支）

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
            if(chara->dir==0)       _priest_update_position(self, -1*move_dis*space_co, 0);
            else if(chara->dir==1)  _priest_update_position(self, move_dis*space_co, 0);
            else if(chara->dir==2)  _priest_update_position(self, 0, -1*move_dis*space_co);
            else if(chara->dir==3)  _priest_update_position(self, 0, move_dis*space_co);
            space = 1;
        }
        else if (DC->key_state[ALLEGRO_KEY_A])
        {
            chara->dir = 0;
            _priest_update_position(self, -1*move_dis, 0);
        }
        else if (DC->key_state[ALLEGRO_KEY_D])
        {
            chara->dir = 1;
            _priest_update_position(self, move_dis, 0);
        }
        else if (DC->key_state[ALLEGRO_KEY_W])
        {
            chara->dir = 2;
            _priest_update_position(self, 0, -1*move_dis);
        }
        else if (DC->key_state[ALLEGRO_KEY_S])
        {
            chara->dir = 3;
            _priest_update_position(self, 0, move_dis);
        }
        else
        {
            chara->state = STOP;
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

            // ★ E技能改成：把所有怪物拉到前面
            _priest_pull_all_monsters_to_front(chara);

            chara->new_proj = true;
        }
    }

    // 其他狀態（ATK/COMBAT）如果你也要保留行為，可以照 susu 那份補齊；
    // 你目前需求只改 Q/E，所以我先保留結構但不強行新增功能。
}

void priest_draw(Elements *self)
{
    // ★ 非操作中的角色不畫出來（數值仍保留）
    if (gControlledCharacter != 3)
        return;

    // with the state, draw corresponding image
    priest *chara = ((priest *)(self->entity));
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
}

void priest_destroy(Elements *self)
{
    if (!self || !self->entity) return;

    priest *Obj = static_cast<priest *>(self->entity);

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

    singleton_priest = nullptr;  // 單例清掉
}

void _priest_update_position(Elements *self, int dx, int dy)
{
    priest *chara = static_cast<priest *>(self->entity);

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

void priest_interact(Elements *self) {}
