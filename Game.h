#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <allegro5/allegro.h>

/**
 * @brief 遊戲主控類別
 * @details 這個 class 就是原本的 gamewindow.c 的封裝版本。
 *          邏輯：初始化 Allegro -> 建立場景 -> 事件迴圈 -> 更新/繪圖 -> 收尾。
 *          全域輸入狀態改由 DataCenter 管理。
 */
class Game
{
public:
    Game();             // 建構子：做原本的 game_init()
    ~Game();            // 解構子：做原本的 game_destroy()

    void execute();     // 主遊戲迴圈（原本的 execute(Game *self)）

private:
    void game_init();   // 原本的 game_init(Game *self)
    bool game_update(); // 原本的 game_update(Game *self)
    void game_draw();   // 原本的 game_draw(Game *self)
    void game_destroy();// 原本的 game_destroy(Game *self)

private:
    const char *title;          // 視窗標題（原本 struct _GAME 的 title）
    ALLEGRO_DISPLAY *display;   // 視窗指標

    // 這兩個原本在 global.h，現在變成 Game 內部成員
    ALLEGRO_TIMER *timer;              // 原本的 fps
    ALLEGRO_EVENT_QUEUE *event_queue;  // 原本的 event_queue
    ALLEGRO_EVENT event;               // 原本的全域 event
};

#endif // GAME_H_INCLUDED
