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
#include <cstddef>
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
    //Destroy();
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

    // ① 先更新死亡 / 過關狀態（只設 flag，不切場景）
    UpdateLevelState();

    // ② 如果玩家已經死亡或過關：停住遊戲，只處理 Enter
    if (is_dead)
    {
        HandleDead();           // 會在按 Enter 時呼叫 ReturnToMenuAfterStage
        UpdatePreviousInputs();
        return;
    }
    if (is_win)
    {
        HandleWin();            // 會在按 Enter 時呼叫 ReturnToMenuAfterStage
        UpdatePreviousInputs();
        return;
    }

    // ③ 如果已經被要求切換場景（例如暫停選單選 Reset / Main Menu）
    if (scene_end)
    {
        std::printf("Scene end triggered, exiting Update\n");
        UpdatePreviousInputs();
        return;
    }

    // ④ 暫停選單處理（ESC / ↑ / ↓ / Enter）
    HandlePause();
    if (scene_end)
    {
        // 暫停選單選了 Reset / Main Menu
        UpdatePreviousInputs();
        return;
    }
    if (is_paused)
    {
        // 暫停中：不更新遊戲邏輯，只維持畫面 + overlay
        UpdatePreviousInputs();
        return;
    }

    // ⑤ 正常遊戲邏輯：時間 / 關卡 / 元件
    double now = al_get_time();
    if (prev_time == 0.0)
        prev_time = now;

    // 按住 RIGHT 快轉（沿用原 C 版的設計）
    if (dc->key_state[ALLEGRO_KEY_RIGHT])
        now += 10.0;

    delta_time = now - prev_time;
    prev_time  = now;

    // 關卡事件（生成怪物、切換 level_no 等）
    Level_switch_Update(this, delta_time);
    // 若之後需要，可恢復：
    // MF_Update(this, delta_time);

    // 讓 Scene 處理所有 Elements 的 Update / Interact / 刪除
    Scene::Update();

    // ⑥ 更新 prev_key / prev_mouse（用來判斷「按一下」）
    UpdatePreviousInputs();
}

/*------------------------------------------------------------
 *  Draw：畫面繪製
 *-----------------------------------------------------------*/

void GameScene::Draw()
{
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
    background      = nullptr;
    game_background = nullptr;
    floor_tile      = nullptr;
    wall_tile       = nullptr;
    pause_font      = nullptr;
    std::printf("[GameScene::Destroy] begin\n");

    std::printf("  [GameScene] objects before cleanup: %zu\n", Objects().size());

    std::printf("  before CleanupElements\n");
    CleanupElements();
    std::printf("  after CleanupElements\n");

    std::printf("  before Level_switch_Destroy\n");
    Level_switch_Destroy();
    std::printf("  after Level_switch_Destroy\n");

    std::printf("  before MF_Destroy\n");
    MF_Destroy();
    std::printf("  after MF_Destroy\n");

    std::printf("  before Scene::Destroy\n");
    //Scene::Destroy();
    std::printf("  after Scene::Destroy\n");

    std::printf("[GameScene::Destroy] end\n");
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
    std::printf("[CleanupElements] begin, count=%zu\n", objs.size());
    std::size_t idx = 0;
    for (Elements *&ele : objs)
    {
        if (!ele)
        {
            std::printf("  #%zu skip (null pointer)\n", idx);
            ++idx;
            continue;
        }

        std::printf("  #%zu label=%d dele=%d entity=%p destroy=%p\n",
                    idx,
                    ele->label,
                    static_cast<int>(ele->dele),
                    ele->entity,
                    reinterpret_cast<void *>(ele->Destroy));

        Elements *target = ele;
        ele = nullptr;

        if (target->Destroy)
        {
            std::printf("    call Destroy for #%zu\n", idx);
            target->Destroy(target);   // 元件自行釋放 entity 內資源
            std::printf("    Destroy done for #%zu\n", idx);
        }
        else if (target->entity)
        {
            // 沒有自訂 Destroy 的，照舊用 C 風格 free entity
            free(target->entity);
            target->entity = nullptr;
        }

        delete target;   // Elements wrapper 一律改由此處 delete

        std::printf("  #%zu done\n", idx);
        ++idx;
    }
    objs.clear();   //  很重要，避免之後又拿這些 dangling pointer 來用
    std::printf("[CleanupElements] end\n");
}



/*------------------------------------------------------------
 *  回主選單（死亡 / 勝利後按 Enter）
 *-----------------------------------------------------------*/

void GameScene::ReturnToMenuAfterStage()//susu *chara)
{
    DataCenter *dc = DataCenter::get_instance();
    (void)dc; // 目前沒用到，保留給之後擴充

    // if (chara)
    //     chara->base.hp = chara->base.full_hp;  // 回主選單時幫玩家補滿血

    for (int i = 0; i < 5; ++i)
        switch_level[i] = 0;                   // 關卡切換 flag 清空

    // 把狀態歸零，避免下次進來還殘留
    is_dead      = false;
    is_win       = false;
    is_paused    = false;
    pause_option = 0;

    SetNextSceneLabel(Menu_L);  // 告訴 SceneManager 下一個場景是主選單
    scene_end = true;           // 通知外面這個 GameScene 可以結束了
}

/*------------------------------------------------------------
 *  Private：死亡 / 過關狀態更新（不直接切場景）
 *-----------------------------------------------------------*/

void GameScene::UpdateLevelState()
{
    Elements *player = get_susu();
    susu *chara      = nullptr;
    if (player && player->entity)
        chara = static_cast<susu *>(player->entity);

    // 只在還沒死、還沒贏的狀態下更新，避免重複判斷
    if (!is_dead && !is_win)
    {
        // 判斷是否過關
        if (is_over())
            is_win = true;

        // 判斷玩家是否死亡
        if (chara && chara->base.hp <= 0)
            is_dead = true;
    }
}

/*------------------------------------------------------------
 *  Private：三個「狀態處理」函數
 *-----------------------------------------------------------*/

// 1. 暫停（ESC / ↑ / ↓ / Enter）
void GameScene::HandlePause()
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

// 2. 死亡狀態處理（顯示死亡畫面，按 Enter 回主選單）
void GameScene::HandleDead()
{
    DataCenter *dc = DataCenter::get_instance();

    // 這裡改成「只要此刻有按著 Enter 就觸發」
    // 允許「長按或單擊 Enter」都能觸發，但避免 scene_end 已經設定時重複執行
    bool enter_down        = dc->key_state[ALLEGRO_KEY_ENTER];
    bool enter_just_pressed = enter_down && !dc->prev_key_state[ALLEGRO_KEY_ENTER];
    if (!enter_down && !enter_just_pressed)
        return;
    if (scene_end)
        return;

    //DataCenter *dc = DataCenter::get_instance();
    (void)dc; // 目前沒用到，保留給之後擴充

    // if (chara)
    //     chara->base.hp = chara->base.full_hp;  // 回主選單時幫玩家補滿血

    for (int i = 0; i < 5; ++i)
        switch_level[i] = 0;                   // 關卡切換 flag 清空

    // 把狀態歸零，避免下次進來還殘留
    is_dead      = false;
    is_win       = false;
    is_paused    = false;
    pause_option = 0;
    SetNextSceneLabel(Menu_L);  // 告訴 SceneManager 下一個場景是主選單
    scene_end = true;           // 通知外面這個 GameScene 可以結束了

    // Elements *player = get_susu();
    // susu *chara = (player && player->entity)
    //                 ? static_cast<susu *>(player->entity)
    //                 : nullptr;

    //ReturnToMenuAfterStage();//chara);
}


// 3. 勝利狀態處理（顯示勝利畫面，按 Enter 回主選單）
void GameScene::HandleWin()
{
    DataCenter *dc = DataCenter::get_instance();
    // 同樣：只要有按著 Enter 就觸發
    bool enter_pressed =
        dc->key_state[ALLEGRO_KEY_ENTER] &&
        !dc->prev_key_state[ALLEGRO_KEY_ENTER];
    if (!enter_pressed)
    {
        std::printf("Enter not pressed\n");
        return;
    }
        
    if (scene_end)
        return;
    if(enter_pressed)
    {
        std::printf("Enter pressed, returning to menu\n");

        // if (chara)
        //     chara->base.hp = chara->base.full_hp;  // 回主選單時幫玩家補滿血

        for (int i = 0; i < 5; ++i)
            switch_level[i] = 0;                   // 關卡切換 flag 清空

        // 把狀態歸零，避免下次進來還殘留
        is_dead      = false;
        is_win       = false;
        is_paused    = false;
        pause_option = 0;
        SetNextSceneLabel(Menu_L);  // 告訴 SceneManager 下一個場景是主選單
        scene_end = true;           // 通知外面這個 GameScene 可以結束了
    }
    //ReturnToMenuAfterStage();

    // Elements *player = get_susu();
    // susu *chara = (player && player->entity)
    //                 ? static_cast<susu *>(player->entity)
    //                 : nullptr;
}


/*------------------------------------------------------------
 *  Private：地圖 / tile 載入
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
    // 關卡 HUD（由 level_switch 系統畫）
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
