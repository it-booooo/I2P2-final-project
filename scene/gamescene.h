#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "scene.h"

class GameScene : public Scene
{
public:
    GameScene();
    ~GameScene();

    void Init();
    void Update();
    void Draw();
    void Destroy();

private:
    double delta_time;

    void ResetInputStates();
    void InitializeElements();
    void CleanupElements();
    void UpdateLevelState();
    void UpdatePreviousInputs();
    void DrawOverlay();
};

#endif /* GAMESCENE_H */
