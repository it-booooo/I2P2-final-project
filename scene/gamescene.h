#ifndef GAMESCENE_H_INCLUDED
#define GAMESCENE_H_INCLUDED

#include "scene.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

struct susu;

/*
 * OOP 版 GameScene：
 * - 繼承 Scene
 * - 搬入：暫停選單、死亡 / 過關畫面、背景切換
 * - 不再依賴 global.h（不再用 window、MAP_WIDTH/HEIGHT、map、floor_tile/wall_tile 全域）
 */
class GameScene : public Scene
{
public:
    GameScene();
    ~GameScene() override;

    void Init() override;
    void Update() override;
    void Draw() override;
    void Destroy() override;

private:
    // 初始化 / 資源管理
    void ResetInputStates();
    void InitializeElements();
    void CleanupElements();
    void LoadMapAndGenerateTiles();   // 目前只載 tile 圖，不讀 map
    void init_resources();

    // 遊戲狀態更新
    void UpdateLevelState();
    void HandlePauseMenu();
    void UpdatePreviousInputs();
    void UpdateBackgroundByLevel();
    void ReturnToMenuAfterStage(susu *chara);

    // 繪製額外畫面：HUD / 暫停 / 死亡 / 過關
    void DrawOverlay();
    void DrawPauseOverlay();
    void DrawDeathOverlay();
    void DrawWinOverlay();

private:
    ALLEGRO_BITMAP *background;      // stage.jpg（目前不一定有用）
    ALLEGRO_BITMAP *game_background; // 依 level_no 切換的背景圖
    ALLEGRO_BITMAP *floor_tile;      // 如要畫 tile map 用，暫時只載圖
    ALLEGRO_BITMAP *wall_tile;
    ALLEGRO_FONT   *pause_font;      // 暫停 / 死亡 / 過關文字字型

    bool  is_paused;
    bool  is_dead;
    bool  is_win;
    int   pause_option;              // 0 = Continue, 1 = Reset, 2 = Main Menu

    double prev_time;
    double delta_time;

    int switch_level[5];             // 紀錄各關卡背景是否已載入
};

#endif
