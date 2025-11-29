#ifndef LEVEL__SWITCH_H
#define LEVEL__SWITCH_H
#include "../scene/scene.h"



typedef enum { LVL_RUNNING, LVL_TRANSITION, LVL_FINISHED } LevelState;int level_no; 
bool is_over();
void Level_switch_Init(void);
void Level_switch_Update(Scene *scene, double dt);
void Level_switch_DrawOverlay(void);
LevelState Level_switch_GetState(void);
void Level_switch_Destroy(void);

#endif