#ifndef CHARATER_H_INCLUDED
#define CHARATER_H_INCLUDED

#include "element.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include "state.h"

// 如果你 cpp 會用到 SoundCenter（susu-style），通常不用在 header include 它
// #include "../data/SoundCenter.h"  // 可選：不建議在 header include 太多

/*
   [character object]
*/
class Character : public Element
{
public:
    int x, y;
    int width, height;               // the width and height of image
    int dir;                         // 1:right, 0:left, 2:up, 3:down
    int state;                       // STOP / MOVE / ATK

    ALGIF_ANIMATION *gif_status[3];  // 0: stop, 1: move, 2: attack

    // ===== susu-style animation control =====
    int  anime;                      // current animation counter
    int  anime_time;                 // duration (frames) for current animation
    bool new_proj;                   // projectile spawned flag (one-shot)
    bool atk_sound_played;           // prevent playing sound every frame

    Shape *hitbox;                   // hitbox of object

    Character()
        : x(0), y(0), width(0), height(0),
          dir(0), state(0),
          gif_status{nullptr, nullptr, nullptr},
          anime(0), anime_time(0),
          new_proj(false), atk_sound_played(false),
          hitbox(nullptr)
    {}
};

Elements *New_Character(int label);
void Character_update(Elements *self);
void Character_interact(Elements *self);
void Character_draw(Elements *self);

// ✅ 統一拼字：destroy
void Character_destroy(Elements *self);

// （可選）相容舊拼字，避免你其他檔案還在呼叫 Character_destory
inline void Character_destory(Elements *self) { Character_destroy(self); }

void _Character_update_position(Elements *self, int dx, int dy);

#endif
