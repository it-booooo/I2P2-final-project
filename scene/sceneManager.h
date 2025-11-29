#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "scene.h"

extern Scene *scene;

void _Scene_init(Scene *target);
void _Scene_destroy(Scene *target);
ElementVec _Get_label_elements(Scene *target, int label);
ElementVec _Get_all_elements(Scene *target);
void _Register_elements(Scene *target, Elements *ele);

#endif /* SCENEMANAGER_H */
