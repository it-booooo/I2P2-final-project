#include "menuscene.h"
#include "sceneManager.h"
#include "../data/DataCenter.h"
#include "../data/FontCenter.h"

#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>

MenuScene::MenuScene()
{
    title_font = nullptr;
    info_font = nullptr;
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

    al_clear_to_color(al_color_name("black"));

    if (title_font)
    {
        al_draw_text(title_font, al_color_name("white"), center_x, start_y, ALLEGRO_ALIGN_CENTRE, "Into the Loop");
    }

    if (info_font)
    {
        float line_gap = 60.0f;
        al_draw_text(info_font, al_color_name("lightgray"), center_x, start_y + line_gap, ALLEGRO_ALIGN_CENTRE, "Press Enter to start");
        al_draw_text(info_font, al_color_name("lightgray"), center_x, start_y + 2.0f * line_gap, ALLEGRO_ALIGN_CENTRE, "Press ESC to quit");
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
