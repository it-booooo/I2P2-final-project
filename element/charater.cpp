#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include "charater.h"
#include "projectile.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../shapes/ShapeFactory.h"
#include "../algif5/algif.h"
#include "../scene/gamescene.h"
#include <cstdio>
/*
   [Character function]
*/
Elements *New_Character(int label)
{
    Character *entity = (Character *)malloc(sizeof(Character));
    Elements *pObj = New_Elements(label);
    // setting derived object member
    // load character images
    char state_string[3][10] = {"stop", "move", "attack"};
    for (int i = 0; i < 3; i++)
    {
        char buffer[50];
        sprintf(buffer, "assets/image/chara_%s.gif", state_string[i]);
        entity->gif_status[i] = algif_load_animation(buffer);
    }
    // load effective sound
    ALLEGRO_SAMPLE *sample = al_load_sample("assets/sound/atk_sound.wav");
    entity->atk_Sound = al_create_sample_instance(sample);
    al_set_sample_instance_playmode(entity->atk_Sound, ALLEGRO_PLAYMODE_ONCE);
    al_attach_sample_instance_to_mixer(entity->atk_Sound, al_get_default_mixer());

    // initial the geometric information of character
    entity->width = entity->gif_status[0]->width;
    entity->height = entity->gif_status[0]->height;
    entity->x = 300;
    entity->y = DataCenter::HEIGHT - entity->height - 60;
    entity->hitbox = New_Rectangle(entity->x,
                                   entity->y,
                                   entity->x + entity->width,
                                   entity->y + entity->height);
    entity->dir = false; // true: face to right, false: face to left
    // initial the animation component
    entity->state = STOP;
    entity->new_proj = false;
    pObj->entity = entity;
    // setting derived object function
    pObj->Draw = Character_draw;
    pObj->Update = Character_update;
    pObj->Interact = Character_interact;
    pObj->Destroy = Character_destory;
    return pObj;
}
void Character_update(Elements *self)
{
    /*// use the idea of finite state machine to deal with different state
    Character *chara = ((Character *)(self->entity));
    if (chara->state == STOP)
    {
        if (key_state[ALLEGRO_KEY_SPACE])
        {
            chara->state = ATK;
        }
        else if (key_state[ALLEGRO_KEY_A])
        {
            chara->dir = 0;
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_D])
        {
            chara->dir = 1;
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_W])
        {
            chara->dir = 2;
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_S])
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
        if (key_state[ALLEGRO_KEY_SPACE])
        {
            chara->state = ATK;
        }
        else if (key_state[ALLEGRO_KEY_A])
        {
            chara->dir = 0;
            _Character_update_position(self, -5, 0);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_D])
        {
            chara->dir = 1;
            _Character_update_position(self, 5, 0);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_W])
        {
            chara->dir = 2;
            _Character_update_position(self, 0, -5);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_S])
        {
            chara->dir = 3;
            _Character_update_position(self, 0, 5);
            chara->state = MOVE;
        }
        if (chara->gif_status[chara->state]->done)
            chara->state = STOP;
    }
    else if (chara->state == ATK)
    {
        if (chara->gif_status[chara->state]->done)
        {
            chara->state = STOP;
            chara->new_proj = false;
        }
        if (chara->gif_status[ATK]->display_index == 2 && chara->new_proj == false)
        {
            Elements *pro;
            if (chara->dir)
            {
                pro = New_Projectile(Projectile_L,
                                     chara->x + chara->width - 100,
                                     chara->y + 10,
                                     5);
            }
            else
            {
                pro = New_Projectile(Projectile_L,
                                     chara->x - 50,
                                     chara->y + 10,
                                     -5);
            }
            _Register_elements(scene, pro);
            chara->new_proj = true;
        }
    }*/
}
void Character_draw(Elements *self)
{
    // with the state, draw corresponding image
    Character *chara = ((Character *)(self->entity));
    ALLEGRO_BITMAP *frame = algif_get_bitmap(chara->gif_status[chara->state], al_get_time());
    if (frame)
    {
        al_draw_bitmap(frame, chara->x, chara->y, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
    }
    if (chara->state == ATK && chara->gif_status[chara->state]->display_index == 2)
    {
        al_play_sample_instance(chara->atk_Sound);
    }
}
void Character_destory(Elements *self)
{
    Character *Obj = ((Character *)(self->entity));
    al_destroy_sample_instance(Obj->atk_Sound);
    for (int i = 0; i < 3; i++)
        algif_destroy_animation(Obj->gif_status[i]);
    delete Obj->hitbox;
    free(Obj);
    free(self);
}

void _Character_update_position(Elements *self, int dx, int dy)
{
    Character *chara = ((Character *)(self->entity));
    chara->x += dx;
    chara->y += dy;
    Shape *hitbox = chara->hitbox;
    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);
}

void Character_interact(Elements *self) {}
