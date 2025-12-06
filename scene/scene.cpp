#include "scene.h"

ElementVec::ElementVec(Elements **elements, int length)
{
    arr = elements;
    len = length;
}

Scene::Scene() : scene_end(false)
{
    next_scene_label = 0;
}

Scene::~Scene()
{
    //Destroy();
}

void Scene::Init()
{
    objects.clear();
    buffer.clear();
}

void Scene::SetNextSceneLabel(int label)
{
    next_scene_label = label;
}

int Scene::NextSceneLabel() const
{
    return next_scene_label;
}

void Scene::Update()
{
    for (Elements *ele : objects)
    {
        if (!ele || ele->dele) continue;     // ★ 新增：刪除標記的不再更新

        if (ele->Update)
            ele->Update(ele);

        if (ele->Interact)                  // ★ 新增：每幀呼叫 Interact
            ele->Interact(ele);
    }
}


void Scene::Draw()
{
    for (Elements *ele : objects)
    {
        if (ele && !ele->dele && ele->Draw)   // ★ 多了 !ele->dele
        {
            ele->Draw(ele);
        }
    }
}


void Scene::Destroy()
{
    for (Elements *&ele : objects)
    {
        if (!ele) continue;

        Elements *target = ele;
        ele = nullptr;

        if (target->Destroy)
        {
            // 元素自己會負責釋放 entity 內部資源
            target->Destroy(target);
        }
        else if (target->entity)
        {
            // 萬一沒實作 Destroy，至少把 entity free 掉避免 leak
            free(target->entity);
            target->entity = nullptr;
        }

        delete target;
    }

    objects.clear();
    buffer.clear();

    std::printf("[Scene::Destroy] end\n");
}


ElementVec Scene::GetLabelElements(int label)
{
    buffer.clear();
    FilterElementsByLabel(label, buffer);
    return BuildElementVec(buffer);
}

ElementVec Scene::GetAllElements()
{
    buffer.clear();
    CollectAllActive(buffer);
    return BuildElementVec(buffer);
}

void Scene::RegisterElement(Elements *ele)
{
    if (ele)
    {
        objects.push_back(ele);
        ele->id = static_cast<int>(objects.size()) - 1;
    }
}

std::vector<Elements *> &Scene::Objects()
{
    return objects;
}

ElementVec Scene::BuildElementVec(std::vector<Elements *> &source)
{
    return ElementVec(source.data(), static_cast<int>(source.size()));
}

void Scene::FilterElementsByLabel(int label, std::vector<Elements *> &collector)
{
    for (Elements *ele : objects)
    {
        if (ele && !ele->dele && ele->label == label)
        {
            collector.push_back(ele);
        }
    }
}

void Scene::CollectAllActive(std::vector<Elements *> &collector)
{
    for (Elements *ele : objects)
    {
        if (ele && !ele->dele)
        {
            collector.push_back(ele);
        }
    }
}
