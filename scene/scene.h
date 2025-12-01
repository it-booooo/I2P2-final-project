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

class ElementVec {
public:
    ElementVec(Elements **elements, int length);

public:
    Elements **arr;
    int len;
};

class Scene {
public:
    Scene();
    ~Scene();

    void Init();
    void Update();
    void Draw();
    void Destroy();
    ElementVec GetLabelElements(int label);
    ElementVec GetAllElements();
    void RegisterElement(Elements *ele);
    std::vector<Elements *> &Objects();

    bool scene_end;

private:
    ElementVec BuildElementVec(std::vector<Elements *> &source);
    void FilterElementsByLabel(int label, std::vector<Elements *> &collector);
    void CollectAllActive(std::vector<Elements *> &collector);

private:
    std::vector<Elements *> objects;
    std::vector<Elements *> buffer;
};

#endif /* SCENE_H */
