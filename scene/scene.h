#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "../element/element.h"

/*
 * 遊戲場景中的元素分類標籤。
 */
enum ElementLabel {
    Floor_L = 0,
    Character_L,
    Tree_L,
    Ball_L,
    Atk_L,
    tungtungtung_L,
    Susu_L,
    trippi_troppi_L,
    capuccino_L,
    bananini_L,
    patapim_L,
    tralala_L,
    crocodilo_L,
    bigtung_L,
    Combat_L,
    Earthquake_L,
    Projectile_L,
    Hpbar_L,
    Teleport_L
};

struct ElementVec {
    Elements **arr;
    int len;
};

struct Scene {
    std::vector<Elements *> objects;
};

#endif /* SCENE_H */
