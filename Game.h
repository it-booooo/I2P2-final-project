#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <allegro5/allegro.h>
#include "UI.h"

/**
 * @brief Main class that runs the whole game.
 * @details All game procedures must be processed through this class.
 */
class Game
{
public:
    // 主迴圈
    void execute();

public:
    // testMode = true 時，不建立 display / timer / event_queue，只做 Allegro 初始化
    Game(bool testMode = false);
    ~Game();

    void game_init();
    bool game_update();
    void game_draw();

private:
    /**
     * @brief States of the game process in game_update.
     * @see Game::game_update()
     */
    enum class STATE {
        START, // -> LEVEL
        LEVEL, // -> PAUSE, END
        PAUSE, // -> LEVEL
        END
    };

    STATE state;
    ALLEGRO_EVENT event;
    ALLEGRO_BITMAP *game_icon;
    ALLEGRO_BITMAP *background;

private:
    ALLEGRO_DISPLAY *display;
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *event_queue;
    UI *ui;
};

#endif
