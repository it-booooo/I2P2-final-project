#include "gamescene.h"
#include "sceneManager.h"
#include "../element/floor.h"
#include "../element/tree.h"
#include "../element/susu.h"
#include "../element/hpbar.h"
#include "../element/teleport.h"
#include "../element/monster_factory.h"
#include "../element/level_switch.h"
#include "../data/DataCenter.h"
#include "../element/charater.h"

GameScene::GameScene()
{
    delta_time = 0.0;
}

GameScene::~GameScene()
{
    Destroy();
}

void GameScene::Init()
{
    Scene::Init();
    ResetInputStates();
    delta_time = 1.0 / DataCenter::FPS;
    Level_switch_Init();
    MF_Reset();
    InitializeElements();
}

void GameScene::Update()
{
    Scene::Update();
    Level_switch_Update(this, delta_time);
    UpdateLevelState();
    UpdatePreviousInputs();
}

void GameScene::Draw()
{
    Scene::Draw();
    DrawOverlay();
}

void GameScene::Destroy()
{
    CleanupElements();
    Scene::Destroy();
    Level_switch_Destroy();
    MF_Destroy();
}

void GameScene::ResetInputStates()
{
    DataCenter *dc = DataCenter::get_instance();
    for (int i = 0; i < ALLEGRO_KEY_MAX; ++i)
    {
        dc->key_state[i] = false;
        dc->prev_key_state[i] = false;
    }
    for (int i = 0; i < ALLEGRO_MOUSE_MAX_EXTRA_AXES; ++i)
    {
        dc->mouse_state[i] = false;
        dc->prev_mouse_state[i] = false;
    }
}

void GameScene::InitializeElements()
{
    Elements *floor = New_Floor(Floor_L);
    RegisterElement(floor);

    Elements *tree = New_Tree(Tree_L);
    RegisterElement(tree);

    Elements *player = New_susu(Susu_L);
    RegisterElement(player);

    Elements *hpbar;
    hpbar = nullptr;
    if (player && player->entity)
    {
        Character *chara = static_cast<Character *>(player->entity);
        hpbar = New_Hpbar(Hpbar_L, chara->base.full_hp, chara->base.hp);
    }
    if (hpbar)
    {
        RegisterElement(hpbar);
    }

    Elements *teleport = New_Teleport(Teleport_L);
    RegisterElement(teleport);
}

void GameScene::CleanupElements()
{
    std::vector<Elements *> &objects = Objects();
    for (Elements *ele : objects)
    {
        if (ele && ele->Destroy)
        {
            ele->Destroy(ele);
        }
    }
    objects.clear();
}

void GameScene::UpdateLevelState()
{
    if (is_over())
    {
        scene_end = true;
        return;
    }

    Elements *player = get_susu();
    if (player && player->entity)
    {
        Character *chara = static_cast<Character *>(player->entity);
        if (chara->base.hp <= 0)
        {
            scene_end = true;
        }
    }
}

void GameScene::UpdatePreviousInputs()
{
    DataCenter *dc = DataCenter::get_instance();
    for (int i = 0; i < ALLEGRO_KEY_MAX; ++i)
    {
        dc->prev_key_state[i] = dc->key_state[i];
    }
    for (int i = 0; i < ALLEGRO_MOUSE_MAX_EXTRA_AXES; ++i)
    {
        dc->prev_mouse_state[i] = dc->mouse_state[i];
    }
}

void GameScene::DrawOverlay()
{
    Level_switch_DrawOverlay();
}
