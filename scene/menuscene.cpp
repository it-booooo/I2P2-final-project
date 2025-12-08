#include "menuscene.h"
#include "sceneManager.h"
#include "../data/DataCenter.h"
#include "../data/FontCenter.h"
#include "../data/ImageCenter.h"
#include "../data/SoundCenter.h"

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
    menu_instance = nullptr;
}

MenuScene::~MenuScene()
{
    //Destroy();
}

void MenuScene::Init()
{
    Scene::Init();

    // ==== 字型 ====
    FontCenter::get_instance()->init();
    FontCenter *fc = FontCenter::get_instance();
    title_font = fc->pirulen[FontSize::LARGE];
    info_font  = fc->pirulen[FontSize::SMALL];

    // ==== 圖片 ====
    ImageCenter *ic = ImageCenter::get_instance();
    enter_icon      = ic->get(ImagePath::ENTER_ICON);
    character_image = ic->get(ImagePath::CHARACTER_IMAGE);

    // ==== 音效 ====
    SoundCenter *sc = SoundCenter::get_instance();

    // 通常建議在 Game 初始化時呼叫一次 init()
    // 如果你目前是放在這裡也可以先留著
    sc->init();

    // 直接用 SoundCenter 播放並取得 instance
    menu_instance = sc->play("assets/sound/menu.mp3", ALLEGRO_PLAYMODE_LOOP);
    if (menu_instance)
    {
        // play() 裡面已經有 attach + play
        // 在這裡只需要調設定就好
        al_set_sample_instance_gain(menu_instance, 0.1f); // 調整音量
    }

    SetNextSceneLabel(Menu_L);
}

void MenuScene::Update()
{
    DataCenter *dc = DataCenter::get_instance();

    // 讓 SoundCenter 定期清掉播完的 instance
    SoundCenter::get_instance()->update();

    bool enter_pressed = dc->key_state[ALLEGRO_KEY_ENTER]  && !dc->prev_key_state[ALLEGRO_KEY_ENTER];
    bool esc_pressed   = dc->key_state[ALLEGRO_KEY_ESCAPE] && !dc->prev_key_state[ALLEGRO_KEY_ESCAPE];

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
    float center_x = static_cast<float>(dc->window_width)  / 2.0f;  // 對應 title_x
    float center_y = static_cast<float>(dc->window_height) / 2.0f;  // 對應 title_y

    // 背景圖：等同於
    // al_draw_scaled_bitmap(enter_icon, 0, 0, w, h, 0, 0, WIDTH, HEIGHT, 0);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    if (enter_icon)
    {
        float src_w  = static_cast<float>(al_get_bitmap_width(enter_icon));
        float src_h  = static_cast<float>(al_get_bitmap_height(enter_icon));
        float dest_w = static_cast<float>(dc->window_width);
        float dest_h = static_cast<float>(dc->window_height);
        al_draw_scaled_bitmap(enter_icon,
                              0.0f, 0.0f, src_w, src_h,
                              0.0f, 0.0f, dest_w, dest_h,
                              0);
    }

    // 半透明白色覆蓋整個畫面
    // 對應：
    // al_draw_filled_rectangle(title_x - WIDTH/2, title_y - HEIGHT/2,
    //                          title_x + WIDTH/2, title_y + HEIGHT/2, ...);
    al_draw_filled_rectangle(center_x - dc->window_width  / 2.0f,
                             center_y - dc->window_height / 2.0f,
                             center_x + dc->window_width  / 2.0f,
                             center_y + dc->window_height / 2.0f,
                             al_map_rgba(255, 255, 255, 60));

    // 角色圖：完全照 C 版的位置與縮放
    if (character_image)
    {
        float src_w = static_cast<float>(al_get_bitmap_width(character_image));
        float src_h = static_cast<float>(al_get_bitmap_height(character_image));
        al_draw_scaled_bitmap(character_image,
                              0.0f, 0.0f, src_w, src_h,
                              550.0f, 400.0f, 639.0f, 960.0f,
                              0);
    }

    // 上面那條灰色提示框
    // al_draw_filled_rectangle(title_x - 150, title_y - 30 - 500,
    //                          title_x + 150, title_y + 30 - 500, ...);
    if (info_font)
    {
        float box_w = 300.0f;   // 左 150 右 150
        float box_h = 60.0f;    // 原本是 -30 ~ +30
        float box_y = center_y - 500.0f - box_h / 2.0f;   // 等同於 center_y - 30 - 500

        // 畫灰色框
        // al_draw_filled_rectangle(center_x - box_w / 2.0f,
        //                         box_y,
        //                         center_x + box_w / 2.0f,
        //                         box_y + box_h,
        //                         al_map_rgba(128, 128, 128, 128));

        // 用字體高度來計算 y，讓文字在框內垂直置中
        float line_h = al_get_font_line_height(info_font);
        float text_y = box_y + (box_h - line_h) / 2.0f + line_h-30.0f; 
        // ↑ 因為 y 是 baseline，所以要再加上 line_h

        al_draw_text(info_font,
                    al_map_rgb(255, 255, 255),
                    center_x,
                    text_y,
                    ALLEGRO_ALIGN_CENTRE,
                    "Press 'Enter' to start");
    }

}


void MenuScene::Destroy()
{
    // ==== BGM 部分：交給 SoundCenter 管理記憶體 ====
    if (menu_instance)
    {
        // 只需要停掉，不要 al_destroy_sample_instance()
        // 因為這個 instance 是由 SoundCenter 建立並持有的
        al_stop_sample_instance(menu_instance);
        menu_instance = nullptr;
    }

    // 下面這些是否要 destroy，要看 FontCenter / ImageCenter 是否自己管理
    // 目前先保留你原本的寫法，如果之後有 double free 再一起調整
    enter_icon = nullptr;
    character_image = nullptr;
    title_font = nullptr;
    info_font = nullptr;
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
