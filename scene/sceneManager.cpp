#include "sceneManager.h"
#include <algorithm>

Scene *scene = nullptr;

void _Scene_init(Scene *target)
{
    if (!target) return;
    target->objects.clear();
    scene = target;
}

void _Scene_destroy(Scene *target)
{
    if (!target) return;
    target->objects.clear();
    if (scene == target) scene = nullptr;
}

static ElementVec build_vec(std::vector<Elements *> &source)
{
    return ElementVec{source.data(), static_cast<int>(source.size())};
}

ElementVec _Get_label_elements(Scene *target, int label)
{
    static std::vector<Elements *> buffer;
    buffer.clear();
    if (target) {
        for (Elements *ele : target->objects) {
            if (ele && !ele->dele && ele->label == label) buffer.push_back(ele);
        }
    }
    return build_vec(buffer);
}

ElementVec _Get_all_elements(Scene *target)
{
    static std::vector<Elements *> buffer;
    buffer.clear();
    if (target) {
        for (Elements *ele : target->objects) {
            if (ele && !ele->dele) buffer.push_back(ele);
        }
    }
    return build_vec(buffer);
}

void _Register_elements(Scene *target, Elements *ele)
{
    if (!target || !ele) return;
    target->objects.push_back(ele);
}
