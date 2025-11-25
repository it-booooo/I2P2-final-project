#include "Ball.h"
#include "tree.h"
#include "charater.h"
#include "../shapes/Circle.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_primitives.h"
/*
   [Ball function]
*/
Elements *New_Ball(int label)
{
    Ball *pDerivedObj = (Ball *)malloc(sizeof(Ball));
    Elements *pObj = New_Elements(label);
    // setting derived object member
    pDerivedObj->x = mouse.x;
    pDerivedObj->y = mouse.y;
    pDerivedObj->r = 10;
    pDerivedObj->color = al_map_rgb(255 ,0, 0);
    pDerivedObj->hitbox = New_Circle(pDerivedObj->x,
                                     pDerivedObj->y,
                                     pDerivedObj->r);
    // setting the interact object
    pObj->inter_obj[pObj->inter_len++] = Character_L;
    pObj->inter_obj[pObj->inter_len++] = Tree_L;
    // setting derived object function
    pObj->pDerivedObj = pDerivedObj;
    pObj->Update = Ball_update;
    pObj->Interact = Ball_interact;
    pObj->Draw = Ball_draw;
    pObj->Destroy = Ball_destory;

    return pObj;
}
void Ball_update(Elements *self)
{
    Ball *Obj = ((Ball *)(self->pDerivedObj));
    Shape *hitbox = Obj->hitbox;
    hitbox->update_center_x(hitbox, mouse.x - Obj->x);
    hitbox->update_center_y(hitbox, mouse.y - Obj->y);
    Obj->x=mouse.x;
    Obj->y=mouse.y;
}
void _Ball_update_position(Elements *self, int dx, int dy)
{
    Ball *Obj = ((Ball *)(self->pDerivedObj));
    Obj->x += dx;
    Obj->y += dy;
    Shape *hitbox = Obj->hitbox;
    hitbox->update_center_x(hitbox, dx);
    hitbox->update_center_y(hitbox, dy);
}
void Ball_interact(Elements *self)
{
    for (int j = 0; j < self->inter_len; j++)
    {
        Ball *Obj =((Ball *)(self->pDerivedObj));
        ElementVec labelEle = _Get_label_elements(scene, Character_L);
        Character *Obj1 = (Character *)(labelEle.arr[0]->pDerivedObj);
        labelEle = _Get_label_elements(scene, Tree_L);
        Tree *Obj2 =(Tree *)(labelEle.arr[0]->pDerivedObj);

        if(Obj -> hitbox->overlap(Obj->hitbox, Obj1->hitbox))
        {
            Obj -> color = al_map_rgb(0,255,0);
        }
        else if(Obj -> hitbox->overlap(Obj->hitbox, Obj2->base.hitbox))
        {
            Obj -> color = al_map_rgb(0,0,255);
        }
        else
        {
            Obj -> color = al_map_rgb(255,0,0);
        }
    }
}
void _Ball_interact_Floor(Elements *self, Elements *tar)
{
    /*Ball *Obj = ((Ball *)(self->pDerivedObj));
    if (Obj->x < 0 - Obj->width)
        self->dele = true;
    else if (Obj->x > WIDTH + Obj->width)
        self->dele = true;*/
}
void _Ball_interact_Tree(Elements *self, Elements *tar)
{
    /*Ball *Obj = ((Ball *)(self->pDerivedObj));
    Tree *tree = ((Tree *)(tar->pDerivedObj));
    if (tree->hitbox->overlap(tree->hitbox, Obj->hitbox))
    {
        self->dele = true;
    }*/
}
void Ball_draw(Elements *self)
{
    Ball *Obj = ((Ball *)(self->pDerivedObj));
    al_draw_circle(Obj->x,Obj->y,Obj->r,Obj->color,10);
}
void Ball_destory(Elements *self)
{
    Ball *Obj = ((Ball *)(self->pDerivedObj));
    free(Obj->hitbox);
    free(Obj);
    free(self);
}
