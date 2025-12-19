#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>

#include "charater.h"
#include "projectile.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include "../scene/gamescene.h"
#include "../data/DataCenter.h"

#include <cstdio>

/*
   [Character function]  (C++ / susu-style)
*/

Elements *New_Character(int label)
{
    Character *entity = new Character{};
    Elements *pObj = New_Elements(label);

    // load character GIF via GIFCenter
    const char *state_string[3] = {"stop", "move", "attack"};
    for (int i = 0; i < 3; i++)
    {
        char path[64];
        std::snprintf(path, sizeof(path), "assets/image/chara_%s.gif", state_string[i]);
        entity->gif_status[i] = GIFCenter::get_instance()->get(path);
    }

    entity->width  = entity->gif_status[0]->width;
    entity->height = entity->gif_status[0]->height;
    entity->x = 300;
    entity->y = DataCenter::HEIGHT - entity->height - 60;

    entity->hitbox = New_Rectangle(
        entity->x,
        entity->y,
        entity->x + entity->width,
        entity->y + entity->height
    );

    entity->dir = 0;

    entity->state      = STOP;
    entity->new_proj   = false;
    entity->anime      = 0;
    entity->anime_time = 0;
    entity->atk_sound_played = false;

    pObj->entity   = entity;
    pObj->Draw     = Character_draw;
    pObj->Update   = Character_update;
    pObj->Interact = Character_interact;
    pObj->Destroy  = Character_destroy;

    return pObj;
}

void Character_update(Elements *self)
{
    Character *chara = static_cast<Character *>(self->entity);
    if (!chara) return;

    DataCenter *DC = DataCenter::get_instance();
    const int move_dis = 5;

    // ===== susu-style animation timer =====
    if (chara->anime_time > 0) {
        chara->anime++;
        if (chara->anime >= chara->anime_time) {
            chara->state      = STOP;
            chara->anime      = 0;
            chara->anime_time = 0;
            chara->new_proj   = false;
            chara->atk_sound_played = false;
        }
    }

    if (chara->state == STOP)
    {
        if (DC->key_state[ALLEGRO_KEY_SPACE])
        {
            chara->state = ATK;
            chara->anime = 0;
            chara->anime_time = 20;
            chara->new_proj = false;
            chara->atk_sound_played = false;
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
    else if (chara->state == MOVE)
    {
        if (DC->key_state[ALLEGRO_KEY_SPACE])
        {
            chara->state = ATK;
            chara->anime = 0;
            chara->anime_time = 20;
            chara->new_proj = false;
            chara->atk_sound_played = false;
        }
        else if (DC->key_state[ALLEGRO_KEY_A])
        {
            chara->dir = 0;
            _Character_update_position(self, -move_dis, 0);
        }
        else if (DC->key_state[ALLEGRO_KEY_D])
        {
            chara->dir = 1;
            _Character_update_position(self, move_dis, 0);
        }
        else if (DC->key_state[ALLEGRO_KEY_W])
        {
            chara->dir = 2;
            _Character_update_position(self, 0, -move_dis);
        }
        else if (DC->key_state[ALLEGRO_KEY_S])
        {
            chara->dir = 3;
            _Character_update_position(self, 0, move_dis);
        }
        else
        {
            chara->state = STOP;
        }
    }
    else if (chara->state == ATK)
    {
        if (chara->gif_status[ATK] &&
            chara->gif_status[ATK]->display_index == 2 &&
            chara->new_proj == false)
        {
            Elements *pro = nullptr;
            if (chara->dir) {
                pro = New_Projectile(Projectile_L,
                                     static_cast<int>(chara->x + chara->width - 100),
                                     static_cast<int>(chara->y + 10),
                                     5);
            } else {
                pro = New_Projectile(Projectile_L,
                                     static_cast<int>(chara->x - 50),
                                     static_cast<int>(chara->y + 10),
                                     -5);
            }
            if (pro) sceneManager.RegisterElement(pro);
            chara->new_proj = true;
        }

        // sound once at frame 2
        if (!chara->atk_sound_played &&
            chara->gif_status[ATK] &&
            chara->gif_status[ATK]->display_index == 2)
        {
            SoundCenter::get_instance()->play("assets/sound/atk_sound.wav",
                                              ALLEGRO_PLAYMODE_ONCE);
            chara->atk_sound_played = true;
        }
    }
}

void Character_draw(Elements *self)
{
    Character *chara = static_cast<Character *>(self->entity);
    if (!chara) return;

    ALLEGRO_BITMAP *frame =
        algif_get_bitmap(chara->gif_status[chara->state], al_get_time());

    if (frame) {
        al_draw_bitmap(frame, chara->x, chara->y,
                       (chara->dir ? ALLEGRO_FLIP_HORIZONTAL : 0));
    }
}

void Character_destroy(Elements *self)
{
    if (!self || !self->entity) return;

    Character *Obj = static_cast<Character *>(self->entity);

    // GIF 由 GIFCenter 管理：不要在這裡 destroy
    for (int i = 0; i < 3; ++i) {
        Obj->gif_status[i] = nullptr;
    }

    if (Obj->hitbox) {
        delete Obj->hitbox;
        Obj->hitbox = nullptr;
    }

    delete Obj;
    self->entity = nullptr;
}

void _Character_update_position(Elements *self, int dx, int dy)
{
    Character *chara = static_cast<Character *>(self->entity);
    if (!chara) return;

    chara->x += dx;
    chara->y += dy;

    Shape *hitbox = chara->hitbox;
    if (!hitbox) return;

    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);
}

void Character_interact(Elements *self)
{
    (void)self;
}
