#ifndef MENUSCENE_H
#define MENUSCENE_H

#include "scene.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_bitmap.h>

class MenuScene : public Scene {
public:
    MenuScene();
    ~MenuScene() override;

    void Init() override;
    void Update() override;
    void Draw() override;
    void Destroy() override;

private:
    void UpdatePreviousInputs();

private:
    ALLEGRO_FONT *title_font;
    ALLEGRO_FONT *info_font;
    ALLEGRO_BITMAP *enter_icon;
    ALLEGRO_BITMAP *character_image;
    ALLEGRO_SAMPLE *menu_bgm;
    ALLEGRO_SAMPLE_INSTANCE *menu_instance;
};

#endif
