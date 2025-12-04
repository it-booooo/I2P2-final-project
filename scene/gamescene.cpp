#include "gamescene.h"
#include "sceneManager.h"
#include "../data/FontCenter.h"
#include "../data/ImageCenter.h"

#include "../element/susu.h"
#include "../element/hpbar.h"
#include "../element/level_switch.h"
#include "../element/monster_factory.h"
#include "../data/DataCenter.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>
#include <cstdio>

/*------------------------------------------------------------
 *  建構 / 解構
 *-----------------------------------------------------------*/

GameScene::GameScene()
    : background(nullptr),
      game_background(nullptr),
      floor_tile(nullptr),
      wall_tile(nullptr),
      pause_font(nullptr),
      is_paused(false),
      is_dead(false),
      is_win(false),
      pause_option(0),
      prev_time(0.0),
      delta_time(0.0)
{
    for (int i = 0; i < 5; ++i)
        switch_level[i] = 0;
}

GameScene::~GameScene()
{
    Destroy();
}

/*------------------------------------------------------------
 *  Init：場景初始化
 *-----------------------------------------------------------*/

 void GameScene::init_resources()
 {
    al_init_image_addon();
    al_init_ttf_addon();
    al_init_font_addon();
    FontCenter::get_instance()->init();
 }

void GameScene::Init()
{
    std::printf("GameScene::Init\n");
    Scene::Init();
    GameScene::init_resources();
    is_paused    = false;
    is_dead      = false;
    is_win       = false;
    pause_option = 0;
    prev_time    = 0.0;
    delta_time   = 0.0;
    for (int i = 0; i < 5; ++i)
        switch_level[i] = 0;

    ResetInputStates();
    ImageCenter *ic = ImageCenter::get_instance();

    // 原 C 版 New_GameScene 裡的背景
    background = ic->get(ImagePath::STAGE_BG);
    if (!background)
        std::fprintf(stderr, "Failed to load assets/image/stage.jpg\n");

    // 目前只負責載入 floor_tile / wall_tile
    LoadMapAndGenerateTiles();

    // 初始化 factory / level switch
    Level_switch_Init();
    MF_Reset();
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    // 暫停 / 結束畫面用字型
    pause_font = FontCenter::get_instance()->pirulen[FontSize::LARGE];
    if (!pause_font)
        std::fprintf(stderr, "Failed to load assets/font/pirulen.ttf\n");

    // 註冊 susu + HP bar（跟舊 C 版一致）
    InitializeElements();
}

/*------------------------------------------------------------
 *  Update：每 frame 更新
 *-----------------------------------------------------------*/

void GameScene::Update()
{
    DataCenter *dc = DataCenter::get_instance();

    // 先更新死亡 / 過關狀態（含 Enter 離開場景）
    UpdateLevelState();
    if (scene_end)
    {
        UpdatePreviousInputs();
        return;
    }

    // 暫停選單處理（ESC / 上下 / Enter）
    HandlePauseMenu();
    if (scene_end)   // 暫停選單可能要求 reset / 回主選單
    {
        UpdatePreviousInputs();
        return;
    }
    if (is_paused)
    {
        UpdatePreviousInputs();
        return;
    }

    // 真正遊戲邏輯：時間 / 關卡 / 元件更新
    double now = al_get_time();
    if (prev_time == 0.0)
        prev_time = now;

    // 原 C 版：按住 RIGHT 快轉
    if (dc->key_state[ALLEGRO_KEY_RIGHT])
        now += 10.0;

    delta_time = now - prev_time;
    prev_time  = now;

    // 關卡邏輯（怪物生成 / 關卡事件等）
    Level_switch_Update(this, delta_time);
    // 若之後需要，可恢復：
    // MF_Update(this, delta_time);

    // 讓 Scene 處理所有 Elements 的 Update / Interact / 刪除
    Scene::Update();

    // 更新 prev_key / prev_mouse（用來判斷「按一下」）
    UpdatePreviousInputs();
}

/*------------------------------------------------------------
 *  Draw：畫面繪製
 *-----------------------------------------------------------*/

void GameScene::Draw()
{
    //std::printf("GameScene::Draw\n");
    const int W = DataCenter::WIDTH;
    const int H = DataCenter::HEIGHT;

    // 背景清空
    al_clear_to_color(al_map_rgb(0, 0, 0));

    // 依 level_no 切換背景圖（level_no / is_over 由 level_switch 系統提供）
    UpdateBackgroundByLevel();

    if (game_background)
    {
        int src_w = al_get_bitmap_width(game_background);
        int src_h = al_get_bitmap_height(game_background);
        al_draw_scaled_bitmap(game_background,
                              0, 0, src_w, src_h,
                              0, 0, W, H, 0);
    }

    // 原 C 版的白色透明遮罩
    al_draw_filled_rectangle(0, 0, W, H,
                             al_map_rgba(255, 255, 255, 100));

    // Tile map 畫法原本就被註解掉，如果之後要用，可以在這裡用 floor_tile / wall_tile 畫

    // 基底 Scene 畫所有 Elements
    Scene::Draw();

    // 疊加 HUD / 暫停 / 死亡 / 過關畫面
    DrawOverlay();
}

/*------------------------------------------------------------
 *  Destroy：釋放資源
 *-----------------------------------------------------------*/

void GameScene::Destroy()
{
    // 先 Destroy 所有 Elements
    CleanupElements();

    // 圖片 / 字型已由 ImageCenter / FontCenter 管理
    // 這裡只把指標設成 nullptr，避免誤用
    background      = nullptr;
    game_background = nullptr;
    floor_tile      = nullptr;
    wall_tile       = nullptr;
    pause_font      = nullptr;

    Level_switch_Destroy();
    MF_Destroy();

    Scene::Destroy();
}


/*------------------------------------------------------------
 *  Private：輸入初始化 / 更新
 *-----------------------------------------------------------*/

void GameScene::ResetInputStates()
{
    DataCenter *dc = DataCenter::get_instance();

    for (int i = 0; i < ALLEGRO_KEY_MAX; ++i)
    {
        dc->key_state[i]      = false;
        dc->prev_key_state[i] = false;
    }
    for (int i = 0; i < ALLEGRO_MOUSE_MAX_EXTRA_AXES; ++i)
    {
        dc->mouse_state[i]      = false;
        dc->prev_mouse_state[i] = false;
    }
}

void GameScene::UpdatePreviousInputs()
{
    DataCenter *dc = DataCenter::get_instance();

    for (int i = 0; i < ALLEGRO_KEY_MAX; ++i)
        dc->prev_key_state[i] = dc->key_state[i];

    for (int i = 0; i < ALLEGRO_MOUSE_MAX_EXTRA_AXES; ++i)
        dc->prev_mouse_state[i] = dc->mouse_state[i];
}

/*------------------------------------------------------------
 *  Private：Elements 初始化 / 清理
 *-----------------------------------------------------------*/

void GameScene::InitializeElements()
{
    // 跟 C 版一樣：只註冊 susu + HP bar
    Elements *player = New_susu(Susu_L);
    if (player)
        RegisterElement(player);

    Elements *hpbar = nullptr;
    if (player && player->entity)
    {
        susu *chara = static_cast<susu *>(player->entity);
        hpbar = New_Hpbar(Hpbar_L, chara->base.full_hp, chara->base.hp);
    }
    if (hpbar)
        RegisterElement(hpbar);
}

void GameScene::CleanupElements()
{
    std::vector<Elements *> &objs = Objects();
    for (Elements *ele : objs)
    {
        if (ele && ele->Destroy)
            ele->Destroy(ele);
    }
    objs.clear();
}

void GameScene::ReturnToMenuAfterStage(susu *chara)
{
    DataCenter *dc = DataCenter::get_instance();
    if (chara)
        chara->base.hp = chara->base.full_hp;

    for (int i = 0; i < 5; ++i)
        switch_level[i] = 0;

    is_dead = false;
    is_win  = false;
    SetNextSceneLabel(Menu_L);
    scene_end = true;
}

/*------------------------------------------------------------
 *  Private：死亡 / 過關狀態更新（搬自 C 版）
 *-----------------------------------------------------------*/

void GameScene::UpdateLevelState()
{
    DataCenter *dc = DataCenter::get_instance();

    // 判斷玩家血量
    Elements *player = get_susu();
    susu *chara      = nullptr;
    if (player && player->entity)
        chara = static_cast<susu *>(player->entity);
    
    // 判斷是否過關
    if (is_over())
    {
        is_win = true;
    }

    if (chara && chara->base.hp <= 0)
        is_dead = true;

    if(is_dead || is_win)
    {
        if (dc->key_state[ALLEGRO_KEY_ENTER])
        {
            dc->key_state[ALLEGRO_KEY_ENTER]= false;
            ReturnToMenuAfterStage(chara);
            return;

        }
    }
}

/*------------------------------------------------------------
 *  Private：暫停選單（ESC / 上下 / Enter）
 *-----------------------------------------------------------*/

void GameScene::HandlePauseMenu()
{
    DataCenter *dc = DataCenter::get_instance();

    bool esc_pressed =
        dc->key_state[ALLEGRO_KEY_ESCAPE] &&
        !dc->prev_key_state[ALLEGRO_KEY_ESCAPE];

    if (esc_pressed)
        is_paused = !is_paused;

    if (!is_paused)
        return;

    bool up_pressed =
        dc->key_state[ALLEGRO_KEY_UP] &&
        !dc->prev_key_state[ALLEGRO_KEY_UP];
    bool down_pressed =
        dc->key_state[ALLEGRO_KEY_DOWN] &&
        !dc->prev_key_state[ALLEGRO_KEY_DOWN];
    bool enter_pressed =
        dc->key_state[ALLEGRO_KEY_ENTER] &&
        !dc->prev_key_state[ALLEGRO_KEY_ENTER];

    if (up_pressed)
        pause_option = (pause_option + 2) % 3; // 上一個選項
    if (down_pressed)
        pause_option = (pause_option + 1) % 3; // 下一個選項

    if (!enter_pressed)
        return;

    if (pause_option == 0)
    {
        // Continue
        is_paused = false;
    }
    else if (pause_option == 1)
    {
        // Reset：要求重新開始一個新的 GameScene
        for (int i = 0; i < 5; ++i)
            switch_level[i] = 0;

        is_paused = false;
        SetNextSceneLabel(GameScene_L);
        scene_end = true;   // 外面的 Game / SceneManager 收到後重新 new GameScene
    }
    else if (pause_option == 2)
    {
        // Main Menu：回主選單
        for (int i = 0; i < 5; ++i)
            switch_level[i] = 0;

        is_paused = false;
        SetNextSceneLabel(Menu_L);
        scene_end = true;   // 外面決定切到主選單 scene
    }
}

/*------------------------------------------------------------
 *  Private：地圖 / tile 載入
 *  - 原 C 版會讀 map.txt 填 map[][]，但你現在不用 global.h，
 *    map 也沒有其他地方用，暫時只載入 tile 圖就好。
 *-----------------------------------------------------------*/

void GameScene::LoadMapAndGenerateTiles()
{
    ImageCenter *ic = ImageCenter::get_instance();
    floor_tile = ic->get(ImagePath::FLOOR_TILE);
    wall_tile  = ic->get(ImagePath::WALL_TILE);
    if (!floor_tile || !wall_tile)
        std::fprintf(stderr, "Failed to load tile images\n");
}

/*------------------------------------------------------------
 *  Private：依 level_no 切換背景（仍靠 level_switch 系統提供 level_no）
 *-----------------------------------------------------------*/

void GameScene::UpdateBackgroundByLevel()
{
    ImageCenter *ic = ImageCenter::get_instance();
    auto reload_background = [this, ic](const char *path)
    {
        // 交給 ImageCenter 做 lazy load & 管理生命週期
        game_background = ic->get(path);
        // ic->get() 內部已經有 GAME_ASSERT，失敗會直接報錯終止
        // 如果你想要額外保險，也可以再檢查一次：
        // if (!game_background)
        //     std::fprintf(stderr, "Failed to load %s\n", path);
    };

    // level_no / is_over 由 level_switch 那邊的程式控制
    extern int level_no;

    if (level_no <= 1)
    {
        if (!switch_level[0])
        {
            std::printf("switch level 0\n");
            switch_level[0] = 1;
            reload_background("assets/image/level0.png");
        }
    }
    else if (level_no == 2)
    {
        if (!switch_level[1])
        {
            std::printf("switch level 1\n");
            switch_level[1] = 1;
            reload_background("assets/image/level1.png");
        }
    }
    else if (level_no == 3)
    {
        if (!switch_level[2])
        {
            std::printf("switch level 2\n");
            switch_level[2] = 1;
            reload_background("assets/image/level2.png");
        }
    }
    else if (level_no == 4)
    {
        if (!switch_level[3])
        {
            std::printf("switch level 3\n");
            switch_level[3] = 1;
            reload_background("assets/image/level3.png");
        }
    }
    else if (level_no == 5)
    {
        if (!switch_level[4])
        {
            std::printf("switch level 4\n");
            switch_level[4] = 1;
            reload_background("assets/image/level3.png");
        }
    }
}

/*------------------------------------------------------------
 *  Private：疊加 overlay（Level_switch HUD + 暫停 / 死亡 / 過關）
 *-----------------------------------------------------------*/

void GameScene::DrawOverlay()
{
    // 原 OOP 版只有這行
    Level_switch_DrawOverlay();

    if (is_paused)
        DrawPauseOverlay();

    if (is_dead)
        DrawDeathOverlay();
    else if (is_win)
        DrawWinOverlay();
}

void GameScene::DrawPauseOverlay()
{
    if (!pause_font)
        return;

    const int W = DataCenter::WIDTH;
    const int H = DataCenter::HEIGHT;

    al_draw_filled_rectangle(0, 0, W, H,
                             al_map_rgba(127, 127, 127, 100));

    al_draw_text(pause_font, al_map_rgb(255, 255, 255),
                 W / 2, H / 2 - 80,
                 ALLEGRO_ALIGN_CENTRE, "Paused");

    al_draw_text(
        pause_font,
        (pause_option == 0 ? al_map_rgb(255, 255, 0) : al_map_rgb(200, 200, 200)),
        W / 2, H / 2 + 0,
        ALLEGRO_ALIGN_CENTRE, "Continue");

    al_draw_text(
        pause_font,
        (pause_option == 1 ? al_map_rgb(255, 255, 0) : al_map_rgb(200, 200, 200)),
        W / 2, H / 2 + 60,
        ALLEGRO_ALIGN_CENTRE, "Reset");

    al_draw_text(
        pause_font,
        (pause_option == 2 ? al_map_rgb(255, 255, 0) : al_map_rgb(200, 200, 200)),
        W / 2, H / 2 + 120,
        ALLEGRO_ALIGN_CENTRE, "Main Menu");
}

void GameScene::DrawDeathOverlay()
{
    if (!pause_font)
        return;

    const int W = DataCenter::WIDTH;
    const int H = DataCenter::HEIGHT;

    al_draw_filled_rectangle(0, 0, W, H,
                             al_map_rgba(0, 0, 0, 160));

    al_draw_text(pause_font, al_map_rgb(255, 255, 255),
                 W / 2, H / 2 - 150,
                 ALLEGRO_ALIGN_CENTRE, "HaHa");

    al_draw_text(pause_font, al_map_rgb(255, 255, 255),
                 W / 2, H / 2,
                 ALLEGRO_ALIGN_CENTRE, "Press Enter to return to menu");
}

void GameScene::DrawWinOverlay()
{
    if (!pause_font)
        return;

    const int W = DataCenter::WIDTH;
    const int H = DataCenter::HEIGHT;

    al_draw_filled_rectangle(0, 0, W, H,
                             al_map_rgba(0, 0, 0, 160));

    al_draw_text(pause_font, al_map_rgb(255, 255, 255),
                 W / 2, H / 2 - 150,
                 ALLEGRO_ALIGN_CENTRE, "CONGRATULATION");

    al_draw_text(pause_font, al_map_rgb(255, 255, 255),
                 W / 2, H / 2,
                 ALLEGRO_ALIGN_CENTRE, "Press Enter to return to menu");
}
