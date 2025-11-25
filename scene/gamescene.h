#ifndef GAMESCENE_H_INCLUDED
#define GAMESCENE_H_INCLUDED
#include "scene.h"

/*
   [game scene object]
*/
typedef enum EleType
{
    Floor_L,
    Teleport_L,
    Tree_L,
    Character_L,
    Projectile_L,
    Ball_L,
    Susu_L,
    tungtungtung_L,
    Atk_L,
    trippi_troppi_L,
    Combat_L,
    Hpbar_L,
    capuccino_L,
    Earthquake_L,
    bananini_L,
    patapim_L,
    tralala_L,
    crocodilo_L,
    bigtung_L
} EleType;

typedef struct _GameScene
{
    ALLEGRO_BITMAP *background;

} GameScene;

void Load_Map_And_Generate_Tile(Scene *scene);
Scene *New_GameScene(int label);
void game_scene_update(Scene *self);
void game_scene_draw(Scene *self);
void game_scene_destroy(Scene *self);

#endif