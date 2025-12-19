#include "Ball.h"
#include "tree.h"
#include "charater.h"
#include "../shapes/Circle.h"
#include "../shapes/ShapeFactory.h"
#include "../scene/sceneManager.h"
#include "../data/DataCenter.h"
#include <allegro5/allegro_primitives.h>

/*
   [Ball function]
*/

// forward declaration（避免 _Ball_update_position 未宣告）
static void _Ball_update_position(Elements *self, int dx, int dy);

Elements *New_Ball(int label)
{
    Ball *entity = new Ball{};
    Elements *pObj = New_Elements(label);

    DataCenter *DC = DataCenter::get_instance();

    entity->x = DC->mouse.x;
    entity->y = DC->mouse.y;
    entity->r = 10;
    entity->color = al_map_rgb(255, 0, 0);
    entity->hitbox = New_Circle(entity->x, entity->y, entity->r);

    // 可互動目標
    pObj->inter_obj[pObj->inter_len++] = Character_L;
    pObj->inter_obj[pObj->inter_len++] = Tree_L;

    pObj->entity   = entity;
    pObj->Update   = Ball_update;
    pObj->Interact = Ball_interact;
    pObj->Draw     = Ball_draw;
    pObj->Destroy  = Ball_destroy;

    return pObj;
}

void Ball_update(Elements *self)
{
    Ball *b = static_cast<Ball *>(self->entity);
    if (!b) return;

    DataCenter *DC = DataCenter::get_instance();

    const int new_x = DC->mouse.x;
    const int new_y = DC->mouse.y;
    const int dx = new_x - b->x;
    const int dy = new_y - b->y;

    _Ball_update_position(self, dx, dy);
}

static void _Ball_update_position(Elements *self, int dx, int dy)
{
    Ball *b = static_cast<Ball *>(self->entity);
    if (!b) return;

    b->x += dx;
    b->y += dy;

    Shape *hitbox = b->hitbox;
    if (!hitbox) return;

    const double cx = hitbox->center_x();
    const double cy = hitbox->center_y();
    hitbox->update_center_x(cx + dx);
    hitbox->update_center_y(cy + dy);
}

void Ball_interact(Elements *self)
{
    Ball *b = static_cast<Ball *>(self->entity);
    if (!b) return;

    // 預設紅色
    b->color = al_map_rgb(255, 0, 0);
    if (!b->hitbox) return;

    // 取得 Character（若場上可能多隻，你就改成遍歷 vec.arr）
    Character *chara = nullptr;
    {
        ElementVec vec = sceneManager.GetLabelElements(Character_L);
        if (vec.len > 0 && vec.arr[0] && vec.arr[0]->entity) {
            chara = static_cast<Character *>(vec.arr[0]->entity);
        }
    }

    // 取得 Tree
    Tree *tree = nullptr;
    {
        ElementVec vec = sceneManager.GetLabelElements(Tree_L);
        if (vec.len > 0 && vec.arr[0] && vec.arr[0]->entity) {
            tree = static_cast<Tree *>(vec.arr[0]->entity);
        }
    }

    // 角色 -> 綠
    if (chara && chara->hitbox) {
        if (b->hitbox->overlap(*chara->hitbox)) {
            b->color = al_map_rgb(0, 255, 0);
        }
    }

    // 樹 -> 藍（覆蓋優先）
    if (tree && tree->base.hitbox) {
        if (b->hitbox->overlap(*tree->base.hitbox)) {
            b->color = al_map_rgb(0, 0, 255);
        }
    }
}

void Ball_draw(Elements *self)
{
    Ball *b = static_cast<Ball *>(self->entity);
    if (!b) return;

    al_draw_circle(b->x, b->y, b->r, b->color, 10);
}

void Ball_destroy(Elements *self)
{
    if (!self || !self->entity) return;

    Ball *b = static_cast<Ball *>(self->entity);

    if (b->hitbox) {
        delete b->hitbox;
        b->hitbox = nullptr;
    }

    delete b;
    self->entity = nullptr;

    // 不要 delete/free self，交給 Scene / SceneManager 管
}
