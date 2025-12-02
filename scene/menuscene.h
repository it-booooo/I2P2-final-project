#ifndef MENUSCENE_H
#define MENUSCENE_H

#include "scene.h"
#include <allegro5/allegro_font.h>

class MenuScene : public Scene {
public:
    MenuScene();
    ~MenuScene() override;

    void Init() override;
    void Update() override;
    void Draw() override;

private:
    void UpdatePreviousInputs();

private:
    ALLEGRO_FONT *title_font;
    ALLEGRO_FONT *info_font;
};

#endif
