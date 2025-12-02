#include "menuscene.h"
#include "sceneManager.h"
#include "../data/DataCenter.h"
#include "../data/FontCenter.h"

#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>

MenuScene::MenuScene()
{
    title_font = nullptr;
    info_font = nullptr;
    enter_icon = nullptr;
    character_image = nullptr;
    menu_bgm = nullptr;
    menu_instance = nullptr;
}

MenuScene::~MenuScene()
{
    Destroy();
}

void MenuScene::Init()
{
    Scene::Init();
    FontCenter::get_instance()->init();
    title_font = FontCenter::get_instance()->pirulen[FontSize::LARGE];
    info_font = FontCenter::get_instance()->pirulen[FontSize::MEDIUM];

    enter_icon = al_load_bitmap("assets/image/enter.png");
    character_image = al_load_bitmap("assets/image/chara.png");
    menu_bgm = al_load_sample("assets/sound/menu.mp3");

    if (menu_bgm)
    {
        menu_instance = al_create_sample_instance(menu_bgm);
        if (menu_instance)
        {
            al_set_sample_instance_playmode(menu_instance, ALLEGRO_PLAYMODE_LOOP);
            al_attach_sample_instance_to_mixer(menu_instance, al_get_default_mixer());
            al_set_sample_instance_gain(menu_instance, 0.1f);
        }
    }

    SetNextSceneLabel(Menu_L);
}

void MenuScene::Update()
{
    DataCenter *dc = DataCenter::get_instance();

    bool enter_pressed = dc->key_state[ALLEGRO_KEY_ENTER] && !dc->prev_key_state[ALLEGRO_KEY_ENTER];
    bool esc_pressed = dc->key_state[ALLEGRO_KEY_ESCAPE] && !dc->prev_key_state[ALLEGRO_KEY_ESCAPE];

    if (enter_pressed)
    {
        SetNextSceneLabel(GameScene_L);
        scene_end = true;
    }
    else if (esc_pressed)
    {
        SetNextSceneLabel(-1);
        scene_end = true;
    }

    UpdatePreviousInputs();
}

void MenuScene::Draw()
{
    DataCenter *dc = DataCenter::get_instance();
    float center_x = static_cast<float>(dc->window_width) / 2.0f;
    float start_y = static_cast<float>(dc->window_height) / 3.0f;

    al_clear_to_color(al_map_rgb(0, 0, 0));

    if (enter_icon)
    {
        float src_w = static_cast<float>(al_get_bitmap_width(enter_icon));
        float src_h = static_cast<float>(al_get_bitmap_height(enter_icon));
        float dest_w = static_cast<float>(dc->window_width);
        float dest_h = static_cast<float>(dc->window_height);
        al_draw_scaled_bitmap(enter_icon, 0.0f, 0.0f, src_w, src_h, 0.0f, 0.0f, dest_w, dest_h, 0);
    }

    float overlay_left = 0.0f;
    float overlay_top = 0.0f;
    float overlay_right = static_cast<float>(dc->window_width);
    float overlay_bottom = static_cast<float>(dc->window_height);
    al_draw_filled_rectangle(overlay_left, overlay_top, overlay_right, overlay_bottom, al_map_rgba(255, 255, 255, 60));

    if (character_image)
    {
        float src_w = static_cast<float>(al_get_bitmap_width(character_image));
        float src_h = static_cast<float>(al_get_bitmap_height(character_image));
        al_draw_scaled_bitmap(character_image, 0.0f, 0.0f, src_w, src_h, 550.0f, 400.0f, 639.0f, 960.0f, 0);
    }

    if (title_font)
    {
        al_draw_text(title_font, al_color_name("white"), center_x, start_y, ALLEGRO_ALIGN_CENTRE, "Into the Loop");
    }

    if (info_font)
    {
        float line_gap = 60.0f;
        float hint_box_height = 60.0f;
        float hint_box_y = start_y - static_cast<float>(dc->window_height) * 0.35f;
        al_draw_filled_rectangle(center_x - 150.0f, hint_box_y, center_x + 150.0f, hint_box_y + hint_box_height, al_map_rgba(128, 128, 128, 128));
        al_draw_text(info_font, al_map_rgb(255, 255, 255), center_x, hint_box_y + 15.0f, ALLEGRO_ALIGN_CENTRE, "Press 'Enter' to start");
        al_draw_text(info_font, al_color_name("lightgray"), center_x, start_y + line_gap, ALLEGRO_ALIGN_CENTRE, "Press Enter to start");
        al_draw_text(info_font, al_color_name("lightgray"), center_x, start_y + 2.0f * line_gap, ALLEGRO_ALIGN_CENTRE, "Press ESC to quit");
    }

    if (menu_instance)
    {
        al_play_sample_instance(menu_instance);
    }
}

void MenuScene::Destroy()
{
    if (menu_instance)
    {
        al_destroy_sample_instance(menu_instance);
        menu_instance = nullptr;
    }

    if (menu_bgm)
    {
        al_destroy_sample(menu_bgm);
        menu_bgm = nullptr;
    }

    if (enter_icon)
    {
        al_destroy_bitmap(enter_icon);
        enter_icon = nullptr;
    }

    if (character_image)
    {
        al_destroy_bitmap(character_image);
        character_image = nullptr;
    }
}

void MenuScene::UpdatePreviousInputs()
{
    DataCenter *dc = DataCenter::get_instance();

    for (int i = 0; i < ALLEGRO_KEY_MAX; ++i)
    {
        dc->prev_key_state[i] = dc->key_state[i];
    }
    for (int i = 0; i < ALLEGRO_MOUSE_MAX_EXTRA_AXES; ++i)
    {
        dc->prev_mouse_state[i] = dc->mouse_state[i];
    }
}
