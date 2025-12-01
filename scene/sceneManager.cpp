#include "sceneManager.h"

SceneManager sceneManager;

SceneManager::SceneManager()
{
    currentScene = nullptr;
}

SceneManager::SceneManager(Scene *current)
{
    currentScene = current;
}

void SceneManager::SetScene(Scene *target)
{
    currentScene = target;
}

Scene *SceneManager::GetScene() const
{
    return currentScene;
}

void SceneManager::InitializeScene()
{
    if (currentScene)
    {
        currentScene->Init();
    }
}

void SceneManager::DestroyScene()
{
    if (currentScene)
    {
        currentScene->Destroy();
        currentScene = nullptr;
    }
}

ElementVec SceneManager::GetLabelElements(int label)
{
    if (currentScene)
    {
        return currentScene->GetLabelElements(label);
    }
    return ElementVec(nullptr, 0);
}

ElementVec SceneManager::GetAllElements()
{
    if (currentScene)
    {
        return currentScene->GetAllElements();
    }
    return ElementVec(nullptr, 0);
}

void SceneManager::RegisterElement(Elements *ele)
{
    if (currentScene)
    {
        currentScene->RegisterElement(ele);
    }
}
