#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "shapes/Shape.h"

// include allegro
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

// include scene and following component
#include "scene/sceneManager.h"
#include "scene/gamescene.h"
#include "scene/menuscene.h"

#include <cstdio>

/**
 * @brief 建構子：設定標題、成員初始值，並呼叫 game_init()
 * 對應原本 New_Game() + game_init(Game *self)
 */
Game::Game(bool isTestMode)
    : title("Into the Loop: Monsters of NTHU"),
      display(nullptr),
      timer(nullptr),
      event_queue(nullptr),
      scene(nullptr),
      window(0),
      testMode(isTestMode)
{
    if (!testMode)
    {
        game_init();
    }
}

/**
 * @brief 解構子：對應原本 game_destroy(Game *self)
 */
Game::~Game()
{
    if (!testMode)
    {
        game_destroy();
    }
}

/**
 * @brief 主遊戲迴圈（原本的 execute(Game *self)）
 * 邏輯完全照著你貼的 gamewindow.c 寫，只是 key/mouse 改用 DataCenter。
 */
void Game::execute()
{
    if (testMode)
    {
        return;
    }

    DataCenter *DC = DataCenter::get_instance();

    // main game loop
    bool run = true;
    while (run)
    {
        // process all events here
        al_wait_for_event(event_queue, &event);
        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
        {
            run &= game_update();
            game_draw();
            break;
        }
        case ALLEGRO_EVENT_DISPLAY_CLOSE: // stop game
        {
            run = false;
            break;
        }
        case ALLEGRO_EVENT_KEY_DOWN:
        {
            // 原本: key_state[event.keyboard.keycode] = true;
            DC->key_state[event.keyboard.keycode] = true;
            break;
        }
        case ALLEGRO_EVENT_KEY_UP:
        {
            DC->key_state[event.keyboard.keycode] = false;
            break;
        }
        case ALLEGRO_EVENT_MOUSE_AXES:
        {
            // 原本: mouse.x / mouse.y
            DC->mouse.x = event.mouse.x;
            DC->mouse.y = event.mouse.y;
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
        {
            // 原本: mouse_state[event.mouse.button] = true;
            DC->mouse_state[event.mouse.button] = true;
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
        {
            DC->mouse_state[event.mouse.button] = false;
            break;
        }
        default:
            break;
        }
    }
}

/**
 * @brief 初始化遊戲（原本的 game_init(Game *self)）
 * 這裡把 WIDTH / HEIGHT / FPS 換成 DataCenter 的設定。
 */
void Game::game_init()
{
    if (testMode)
    {
        return;
    }

    DataCenter *DC = DataCenter::get_instance();

    std::printf("Game Initializing...\n");
    GAME_ASSERT(al_init(), "failed to initialize allegro.");

    // initialize allegro addons
    bool addon_init = true;
    addon_init &= al_init_primitives_addon();
    addon_init &= al_init_font_addon();   // initialize the font addon
    addon_init &= al_init_ttf_addon();    // initialize the ttf (True Type Font) addon
    addon_init &= al_init_image_addon();  // initialize the image addon
    addon_init &= al_init_acodec_addon(); // initialize acodec addon
    addon_init &= al_install_keyboard();  // install keyboard event
    addon_init &= al_install_mouse();     // install mouse event
    addon_init &= al_install_audio();     // install audio event
    GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

    // Create display
    // 原本: al_create_display(WIDTH, HEIGHT);
    // 現在用 DataCenter 裡的 window_width / window_height
    display = al_create_display(DC->window_width, DC->window_height);
    GAME_ASSERT(display, "failed to create display.");

    create_scene(Menu_L);

    // create event queue（原本是 global event_queue，現在變成成員）
    event_queue = al_create_event_queue();
    GAME_ASSERT(event_queue, "failed to create event queue.");

    // Initialize Allegro settings
    al_set_window_position(display, 0, 0);
    al_set_window_title(display, title);
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    // Register event
    al_register_event_source(event_queue, al_get_display_event_source(display)); // register display event
    al_register_event_source(event_queue, al_get_keyboard_event_source());       // register keyboard event
    al_register_event_source(event_queue, al_get_mouse_event_source());          // register mouse event

    // register timer event
    // 原本: fps = al_create_timer(1.0 / FPS);
    timer = al_create_timer(1.0 / DataCenter::FPS);
    GAME_ASSERT(timer, "failed to create timer.");
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

    // initialize the icon on the display
    ALLEGRO_BITMAP *icon = al_load_bitmap("assets/image/icon.jpg");
    if (icon)
    {
        al_set_display_icon(display, icon);
        // 若要更嚴格管理資源，可以保留 icon 成員並在 destructor 裡 destroy
        // 這邊先維持跟原本一樣的「不用特別 destroy」行為（由 Allegro 結束時一起清）
    }
}

/**
 * @brief 更新遊戲邏輯（原本的 game_update(Game *self)）
 * @return true 繼續遊戲，false 結束主迴圈
 */
bool Game::game_update()
{
    if (!scene)
    {
        return false;
    }

    scene->Update();
    if (scene->scene_end)
    {
        int target = scene->NextSceneLabel();
        switch (target)
        {
        case Menu_L:
            create_scene(Menu_L);
            break;
        case GameScene_L:
            create_scene(GameScene_L);
            break;
        case -1:
            return false;
        default:
            return false;
        }
    }
    return true;
    //return scene != nullptr;
}

/**
 * @brief 繪製畫面（原本的 game_draw(Game *self)）
 */
void Game::game_draw()
{
    // Flush the screen first.
    al_clear_to_color(al_map_rgb(100, 100, 100));
    if (scene)
    {
        scene->Draw();
    }
    else
    {
        std::printf("No scene to draw!\n");
    }
    al_flip_display();
}

/**
 * @brief 資源釋放（原本的 game_destroy(Game *self)）
 */
void Game::game_destroy()
{
    if (testMode)
    {
        return;
    }

    // Make sure you destroy all things
    if (event_queue)
    {
        al_destroy_event_queue(event_queue);
        event_queue = nullptr;
    }

    if (timer)
    {
        al_destroy_timer(timer);
        timer = nullptr;
    }

    if (display)
    {
        al_destroy_display(display);
        display = nullptr;
    }

    if (scene)
    {
        scene->Destroy();
        delete scene;
        scene = nullptr;
        sceneManager.SetScene(nullptr);
    }
}

void Game::create_scene(int label)
{
    if (scene)
    {
        scene->Destroy();
        delete scene;
        scene = nullptr;
    }

    switch (label)
    {
    case Menu_L:
        scene = new MenuScene();
        break;
    case GameScene_L:
        scene = new GameScene();
        break;
    default:
        scene = new Scene();
        break;
    }

    sceneManager.SetScene(scene);
    scene->SetNextSceneLabel(label);
    sceneManager.InitializeScene();
    window = label;
}
