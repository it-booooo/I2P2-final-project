#ifndef Atk_H_INCLUDED
#define Atk_H_INCLUDED

/* ----------- 外部依賴 ----------- */
//#include <allegro5/allegro_bitmap.h>   /* ALLEGRO_BITMAP */
#include "element.h"
#include "../shapes/Shape.h"

/* ----------- 陣營常數 ----------- */
#define SIDE_PLAYER 0   /* 玩家或友軍子彈／角色 */
#define SIDE_ENEMY  1   /* 敵方子彈／怪物       */

/*
   [Atk object] ― 所有飛行投射物共用
*/
typedef struct _Atk
{
    int x, y;              // 位置
    int width, height;     // 尺寸
    int vx, vy;            // 速度
    int damage;            // 傷害值
    int side;              // 陣營 (SIDE_PLAYER / SIDE_ENEMY)
    ALLEGRO_BITMAP *img;   // 圖片
    Shape *hitbox;         // 碰撞盒
} Atk;

/* ----------- 介面函式 ----------- */

/* 建立一顆新子彈（預設貼圖 ball-02.png） */
Elements *New_Atk(int label,
                  int x, int y,
                  float vx, float vy,
                  int damage, int side);

/* ★動態換子彈圖片（png / jpg） */
void Atk_set_image(Elements *self, const char *img_path);

/* 生命週期函式 */
void Atk_update  (Elements *self);
void Atk_interact(Elements *self);
void Atk_draw    (Elements *self);
void Atk_destory (Elements *self);

/* 供其他模組調用的內部工具（原本已有，保留相容） */
void _Atk_update_position(Elements *self, float dx, float dy);
void _Atk_interact_Floor (Elements *self, Elements *tar);
void _Atk_interact_Tree  (Elements *self, Elements *tar);
void DealDamageIfPossible(Elements *target, int damage);

#endif /* Atk_H_INCLUDED */
