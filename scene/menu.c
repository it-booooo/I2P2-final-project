#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "menu.h"
#include <stdbool.h>

ALLEGRO_BITMAP *enter_icon = NULL;
ALLEGRO_BITMAP *character = NULL;
/*
   [Menu function]
*/
Scene *New_Menu(int label)
{
    Menu *pDerivedObj = (Menu *)malloc(sizeof(Menu));
    Scene *pObj = New_Scene(label);
    // setting derived object member
    pDerivedObj->font = al_load_ttf_font("assets/font/pirulen.ttf", 15, 0);
    // Load sound
    pDerivedObj->song = al_load_sample("assets/sound/menu.mp3");
    al_reserve_samples(20);
    pDerivedObj->sample_instance = al_create_sample_instance(pDerivedObj->song);
    pDerivedObj->title_x = WIDTH / 2;
    pDerivedObj->title_y = HEIGHT / 2;
    // Loop the song until the display closes
    al_set_sample_instance_playmode(pDerivedObj->sample_instance, ALLEGRO_PLAYMODE_LOOP);
    al_restore_default_mixer();
    al_attach_sample_instance_to_mixer(pDerivedObj->sample_instance, al_get_default_mixer());
    // set the volume of instance
    al_set_sample_instance_gain(pDerivedObj->sample_instance, 0.1);
    pObj->pDerivedObj = pDerivedObj;
    // setting derived object function
    pObj->Update = menu_update;
    pObj->Draw = menu_draw;
    pObj->Destroy = menu_destroy;
    return pObj;
}
void menu_update(Scene *self)
{
    if (!enter_icon)
        enter_icon = al_load_bitmap("assets/image/enter.png");
    if(!character)  
        character = al_load_bitmap("assets/image/chara.png");
    if (key_state[ALLEGRO_KEY_ENTER])
    {
        self->scene_end = true;
        window = 1;
    }
    return;
}
void menu_draw(Scene *self)
{
    Menu *Obj = ((Menu *)(self->pDerivedObj));
    al_draw_scaled_bitmap(enter_icon, 0, 0, al_get_bitmap_width(enter_icon), al_get_bitmap_height(enter_icon), 0, 0, WIDTH, HEIGHT, 0);
    al_draw_filled_rectangle(Obj->title_x - WIDTH/2, Obj->title_y - HEIGHT/2, Obj->title_x + WIDTH/2, Obj->title_y + HEIGHT/2, al_map_rgba(255, 255, 255,60));
    al_draw_scaled_bitmap(character, 0, 0, al_get_bitmap_width(character), al_get_bitmap_height(character), 550, 400, 639, 960, 0);
    al_draw_filled_rectangle(Obj->title_x - 150, Obj->title_y - 30-500, Obj->title_x + 150, Obj->title_y + 30-500, al_map_rgba(128, 128, 128, 128));
    al_draw_text(Obj->font, al_map_rgb(255, 255, 255), Obj->title_x, Obj->title_y-505, ALLEGRO_ALIGN_CENTRE, "Press 'Enter' to start");
    al_play_sample_instance(Obj->sample_instance);
}
void menu_destroy(Scene *self)
{
    Menu *Obj = ((Menu *)(self->pDerivedObj));
    al_destroy_font(Obj->font);
    al_destroy_sample(Obj->song);
    al_destroy_sample_instance(Obj->sample_instance);
    free(Obj);
    free(self);
}
