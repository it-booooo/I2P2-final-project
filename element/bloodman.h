#ifndef BLOODMAN_H_INCLUDED
#define BLOODMAN_H_INCLUDED

#include <allegro5/allegro_audio.h>
#include "damageable.h"
#include "element.h"
#include "../shapes/Shape.h"
#include "../algif5/algif.h"
#include "state.h"

// 跟 susu 共用同一個控制變數
extern int gControlledCharacter;

/*
   [Bloodman object]
   角色2：使用你現在改過的技能版本（Q 扣 50% 血，10 秒內打到敵人會吸血，
          Q 沒有投擲物，E 改成丟投擲物，取消地震）
*/
class Bloodman : public Element
{
public:
    Damageable base;
    int x, y;
    int width, height;
    int dir;
    int damage;
    int state;
    ALGIF_ANIMATION *gif_status[5];
    ALLEGRO_SAMPLE_INSTANCE *atk_Sound;
    int anime;
    int anime_time;
    int e_timer, q_timer;
    bool new_proj;
    ALLEGRO_BITMAP *img;

    // Q 技能吸血 buff 狀態
    bool lifesteal_active;
    int  lifesteal_timer;   // 以 frame 計，約 10 秒
};

Elements *New_Bloodman(int label);
void bloodman_update(Elements *self);
void bloodman_interact(Elements *self);
void bloodman_draw(Elements *self);
void bloodman_destroy(Elements *self);
void _bloodman_update_position(Elements *self, int dx, int dy);

// 取得 Bloodman 這個角色2的 Elements*
Elements *get_bloodman(void);

#endif
