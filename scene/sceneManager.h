#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "scene.h"

class SceneManager
{
public:
    SceneManager();
    explicit SceneManager(Scene *current);

    void SetScene(Scene *target);
    Scene *GetScene() const;
    void InitializeScene();
    void DestroyScene();
    ElementVec GetLabelElements(int label);
    ElementVec GetAllElements();
    void RegisterElement(Elements *ele);

private:
    Scene *currentScene;
};

extern SceneManager sceneManager;

#endif /* SCENEMANAGER_H */
