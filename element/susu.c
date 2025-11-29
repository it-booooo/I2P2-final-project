#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include "susu.h"
#include "hpbar.h"
#include "projectile.h"
#include "atk.h"
#include "combat.h"
#include "earthquake.h"
#include "../global.h"
#include "../scene/sceneManager.h"
#include "../shapes/Rectangle.h"
#include "../algif5/algif.h"
#include "../scene/gamescene.h"
#include <allegro5/allegro_native_dialog.h>
#include <stdio.h>
#include <stdbool.h>
#define M_PI 3.14159265358979323846
/*
   [susu function]
*/

static Elements *singleton_susu = NULL; // CHANGED: added singleton pointer to expose susu to other modules

Elements *get_susu(void) // CHANGED: accessor to retrieve the singleton pointer
{
    return singleton_susu;
}

Elements *New_susu(int label)
{
    susu *pDerivedObj = (susu *)malloc(sizeof(susu));
    Elements *pObj = New_Elements(label);
    // setting derived object member
    // load susu images
    char state_string[5][10] = {"stop_2", "move_2", "attack_2","combat_2","earth_2"};
    for (int i = 0; i < 5; i++)
    {
        char buffer[50];
        sprintf(buffer, "assets/image/chara_%s.gif", state_string[i]);
        pDerivedObj->gif_status[i] = algif_new_gif(buffer, -1);
    }
    //pDerivedObj->img = al_load_bitmap("assets/image/susu_1.png");
    // load effective sound
    ALLEGRO_SAMPLE *sample = al_load_sample("assets/sound/atk_sound.wav");
    pDerivedObj->atk_Sound = al_create_sample_instance(sample);
    al_set_sample_instance_playmode(pDerivedObj->atk_Sound, ALLEGRO_PLAYMODE_ONCE);
    al_attach_sample_instance_to_mixer(pDerivedObj->atk_Sound, al_get_default_mixer());




    // initial the geometric information of susu
    pDerivedObj->width = pDerivedObj->gif_status[0]->width;
    pDerivedObj->height = pDerivedObj->gif_status[0]->height;
    pDerivedObj->x = 300;
    pDerivedObj->y = HEIGHT - pDerivedObj->height - 60;
    pDerivedObj->base.hitbox = New_Rectangle(pDerivedObj->x+200,
                                        pDerivedObj->y+50,
                                        pDerivedObj->x + pDerivedObj->width-200,
                                        pDerivedObj->y + pDerivedObj->height-50);
    pDerivedObj->base.hp=10000;                                    
    pDerivedObj->base.full_hp=10000;
    pDerivedObj->base.side=0;
    pDerivedObj->dir = false; // true: face to right, false: face to left
    pDerivedObj->damage = 80;
    // initial the animation component
    pDerivedObj->state = STOP;
    pDerivedObj->new_proj = false;
    pDerivedObj->e_timer = 0;
    pDerivedObj->q_timer = 0;
    pObj->pDerivedObj = pDerivedObj;
    // setting derived object function
    pObj->Draw = susu_draw;
    pObj->Update = susu_update;
    pObj->Interact = susu_interact;
    pObj->Destroy = susu_destroy;

    singleton_susu = pObj; // CHANGED: save pointer to singleton
    return pObj;
}




void susu_update(Elements *self)
{
    // use the idea of finite state machine to deal with different state
    
    susu *chara = ((susu *)(self->pDerivedObj));
    if(chara->e_timer>0)chara->e_timer--;
    if(chara->q_timer>0)chara->q_timer--;
    int move_dis = 10;
    static bool space=0;
    int space_co = 15;
    ALLEGRO_MOUSE_STATE state;
    al_get_mouse_state(&state);
    if (key_state[ALLEGRO_KEY_SPACE]==0)
    {
        space=0;
    }
    if (chara->state == STOP)
    {
        if (state.buttons & 1)
        {
            chara->state = COMBAT;
        }
        else if (key_state[ALLEGRO_KEY_Q])
        {
            if(chara->q_timer <= 0)
            {
                chara->state = ATK;
            }
        }
        else if (key_state[ALLEGRO_KEY_E])
        {
            if(chara->e_timer <=0)
            {
                chara->state = EARTHQUAKE;
            }
        }
        else if (key_state[ALLEGRO_KEY_SPACE] && space==0)
        {
            if(chara->dir==0)  _susu_update_position(self, -1*move_dis*space_co, 0);
            else if(chara->dir==1) _susu_update_position(self, move_dis*space_co, 0);
            else if(chara->dir==2) _susu_update_position(self, 0, -1*move_dis*space_co);
            else if(chara->dir==3) _susu_update_position(self, 0, move_dis*space_co);
            space = 1;
            chara->state = MOVE;
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
        if (state.buttons & 1)
        {
            chara->state = COMBAT;
        }
        else if (key_state[ALLEGRO_KEY_Q])
        {
            if(chara->q_timer <= 0)
            {
                chara->state = ATK;
            }
        }
        else if (key_state[ALLEGRO_KEY_E])
        {
            if(chara->e_timer <=0)
            {
                chara->state = EARTHQUAKE;
            }
        }
        else if (key_state[ALLEGRO_KEY_SPACE] && space==0)
        {
            if(chara->dir==0)  _susu_update_position(self, -1*move_dis*space_co, 0);
            else if(chara->dir==1) _susu_update_position(self, move_dis*space_co, 0);
            else if(chara->dir==2) _susu_update_position(self, 0, -1*move_dis*space_co);
            else if(chara->dir==3) _susu_update_position(self, 0, move_dis*space_co);
            space =1;
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_A])
        {
            chara->dir = 0;
            _susu_update_position(self, -1*move_dis, 0);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_D])
        {
            chara->dir = 1;
            _susu_update_position(self, move_dis, 0);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_W])
        {
            chara->dir = 2;
            _susu_update_position(self, 0, -1*move_dis);
            chara->state = MOVE;
        }
        else if (key_state[ALLEGRO_KEY_S])
        {
            chara->dir = 3;
            _susu_update_position(self, 0, move_dis);
            chara->state = MOVE;
        }
        if (chara->gif_status[chara->state]->done)
        {
            chara->state = STOP;
        }
           
    }
    else if (chara->state == ATK)
    {
        if (chara->gif_status[chara->state]->done)
        {
            chara->state = STOP;
            chara->new_proj = false;
        }
        if (chara->gif_status[ATK]->display_index == 2 && chara->new_proj == false && chara->q_timer <= 0)
        {
            chara->q_timer=60;
            Elements *pro;
            float dx = mouse.x - (chara->x + chara->width*0.5);
            float dy = mouse.y - (chara->y + chara->height*0.5);
            float len = sqrt(dx * dx + dy * dy);
            /*const float base_speed   = 12.0;
            const float extra = 0.1;

            float speed = base_speed + extra * len;
            if (speed > 40.0) speed = 40.0;*/

            float speed = 25.0;
            float vx = speed * dx / len;
            float vy = speed * dy / len;
            pro = New_Atk(Atk_L,chara->x + chara->width*0.5 - 20.0, chara->y + chara->height*0.5 - 70.0,vx,vy,chara->damage*2,0);                                      
            if(pro)
            {
                _Register_elements(scene, pro);
            }


            chara->new_proj = true;
        }
    }
    else if (chara->state == COMBAT)
    {
        if (chara->gif_status[chara->state]->done)
        {
            chara->state = STOP;
            chara->new_proj = false;
        }
        if (chara->gif_status[COMBAT]->display_index == 3 && chara->new_proj == false)
        {
            Elements *pro;
            float dx = mouse.x - (chara->x + chara->width*0.5);
            float dy = mouse.y - (chara->y + chara->height*0.5);

            const int reach = 200;      // 攻擊距離
            const int thick = 300;      // 攻擊寬度

            float cx = chara->x + chara->width  * 0.5f;
            float cy = chara->y + chara->height * 0.5f;

            float angle = atan2f(dy, dx);   // [-π, π]
            int x1,y1,x2,y2;

            // 依角度歸類方向
            int dir;
            if (angle > -M_PI/4 && angle <=  M_PI/4)      dir = 0; // 右
            else if (angle >  M_PI/4 && angle <= 3*M_PI/4) dir = 1; // 下
            else if (angle > -3*M_PI/4 && angle <=-M_PI/4) dir = 2; // 上
            else                                            dir = 3; // 左

            switch (dir) {
                case 0: // → 右
                    x1 = cx;
                    y1 = cy - thick/2;
                    x2 = cx + reach;
                    y2 = cy + thick/2;
                    break;
                case 1: // ↓ 下
                    x1 = cx - thick/2;
                    y1 = cy;
                    x2 = cx + thick/2;
                    y2 = cy + reach;
                    break;
                case 2: // ↑ 上
                    x1 = cx - thick/2;
                    y1 = cy - reach;
                    x2 = cx + thick/2;
                    y2 = cy;
                    break;
                case 3: // ← 左
                    x1 = cx - reach;
                    y1 = cy - thick/2;
                    x2 = cx;
                    y2 = cy + thick/2;
                    break;
            }

            pro = New_Combat(Combat_L, x1, y1, x2, y2, chara->damage, 0);                                      
            if(pro)
            {
                _Register_elements(scene, pro);
            }
            chara->new_proj = true;
        }
    }
    else if(chara->state == EARTHQUAKE)
    {
        if (chara->gif_status[chara->state]->done)
        {
            chara->state = STOP;
            chara->new_proj = false;
        }
        if (chara->gif_status[EARTHQUAKE]->display_index == 4 && chara->new_proj == false   && chara->e_timer <=0)
        {
            chara->e_timer =60;
            Elements *pro;
            pro = New_Earthquake(Earthquake_L,chara->x + chara->width*0.5-192.0, chara->y + chara->height*0.5-100.0, chara->damage, 0);                                      
            if(pro)_Register_elements(scene, pro);
            chara->new_proj = true;
        }
    }
}
void susu_draw(Elements *self)
{
    // with the state, draw corresponding image
    susu *chara = ((susu *)(self->pDerivedObj));
    ALLEGRO_BITMAP *frame = algif_get_bitmap(chara->gif_status[chara->state], al_get_time());
    //al_draw_bitmap(chara->img, chara->x, chara->y, ((chara->dir) ? ALLEGRO_FLIP_HORIZONTAL : 0));
    if (frame)
    {
        al_draw_bitmap(frame, chara->x, chara->y, (((mouse.x - (chara->x + chara->width*0.5))>0) ? ALLEGRO_FLIP_HORIZONTAL : 0));
    }
    if (chara->atk_Sound && chara->gif_status[chara->state] &&chara->state == ATK && chara->gif_status[chara->state]->display_index == 2)
    {
        al_play_sample_instance(chara->atk_Sound);
        //chara->state = STOP;
    }
    /*Elements *hpbar;
    hpbar = New_Hpbar(Hpbar_L,chara->base.full_hp,chara->base.hp);
    _Register_elements(scene,hpbar);*/
}
void susu_destroy(Elements *self)
{
    susu *Obj = ((susu *)(self->pDerivedObj));
    al_destroy_sample_instance(Obj->atk_Sound);
    //al_destroy_bitmap(Obj->img);
    for (int i = 0; i < 3; i++)
        algif_destroy_animation(Obj->gif_status[i]);
    free(Obj->base.hitbox);
    free(Obj);
    free(self);
}




void _susu_update_position(Elements *self, int dx, int dy)
{
    susu *chara = ((susu *)(self->pDerivedObj));
    if(chara->x + chara->width*0.5 == 0 && dx <0) dx =0;
    if(chara->x + chara->width*0.5 ==1800 && dx>0) dx =0;
    if(chara->y + chara->height - 200 == 0 && dy<0) dy =0;
    if(chara->y + chara->height == 1400 && dy>0) dy =0;
    chara->x += dx;
    chara->y += dy;
    Shape *hitbox = chara->base.hitbox;
    hitbox->update_center_x(hitbox, dx);
    hitbox->update_center_y(hitbox, dy);
}

void susu_interact(Elements *self) {}