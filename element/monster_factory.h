#ifndef MONSTER_FACTORY_H
#define MONSTER_FACTORY_H

#include "../scene/scene.h"

/*  Monster kinds supported by the factory */
typedef enum {
    MON_TUNGTUNGTUNG,
    MON_SUSU,
    MON_BALL,
    MON_TRIPPI_TROPPI,
    MON_CAPUCCINO,
    MON_BANANINI,
    MON_PATAPIM,
    MON_TRALALA,
    MON_CROCODILO,
    MON_BIGTUNG
    // add new monsters here
} MonsterType;

/* Spawn one monster of given type at (x,y) if constructor supports coordinates.
   If your monster constructors don’t take positions, (x,y) can be ignored. */
void MF_Spawn(Scene *scene, MonsterType type, float x, float y);

/* Called every frame from GameScene::Update to drive scripted / timed waves. */
void MF_Update(Scene *scene, double delta_sec);

/* Reset internal timers / wave index (e.g., when a new GameScene is created). */
void MF_Reset(void);

/* Free any resources owned by the factory. */
void MF_Destroy(void);

bool MF_NextWaveIsBuffer(void);

void MF_SkipBufferWave(void);

int MF_AliveMonsterCount(Scene *scene);

#endif
