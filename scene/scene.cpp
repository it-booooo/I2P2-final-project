#include "scene.h"

ElementVec::ElementVec(Elements **elements, int length)
{
    arr = elements;
    len = length;
}

Scene::Scene() {}

Scene::~Scene()
{
    Destroy();
}

void Scene::Init()
{
    objects.clear();
    buffer.clear();
}

void Scene::Destroy()
{
    objects.clear();
    buffer.clear();
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
