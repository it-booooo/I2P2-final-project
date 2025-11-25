#ifndef crocodilo_H_INCLUDED          /* ✨ 宏名稱統一大寫 */
#define crocodilo_H_INCLUDED

#include <allegro5/allegro.h>             /* 只需要 ALLEGRO_BITMAP */
#include "element.h"
#include "damageable.h"
#include "../shapes/Shape.h"
#include <stdbool.h>
#include "state.h"                        /* 若 STOP / MOVE / ATK 定義於此 */

typedef struct {
    Damageable      base;                 /* 放第一，Atk.c 轉型用 */
    int             x, y;
    int             width, height;
    bool            dir;                  /* true → face right */
    int             state;                /* STOP / MOVE / ATK */
    ALLEGRO_BITMAP *img[3];               /* 0=stop,1=move,2=attack (png) */
    int             cooldown;             /* 幀倒數：0 代表可射擊 */
} crocodilo;

/* ---- 公開函式 ---- */
Elements *New_crocodilo(int label);
void       crocodilo_update(Elements *self);
void       crocodilo_interact(Elements *self);
void       crocodilo_draw(Elements *self);
void       crocodilo_destory(Elements *self);



#endif /* crocodilo_H_INCLUDED */