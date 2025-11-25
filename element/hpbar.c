#include "hpbar.h"
#include "tree.h"
#include "susu.h"
#include "../shapes/Rectangle.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_primitives.h"
/*
   [Hpbar function]
*/
Elements *New_Hpbar(int label,int full_hp, int now_hp)
{
    Hpbar *pDerivedObj = (Hpbar *)malloc(sizeof(Hpbar));
    Elements *pObj = New_Elements(label);
    // setting derived object member
    pDerivedObj->x = 10;
    pDerivedObj->y = 1300;
    pDerivedObj->full_length=400;
    pDerivedObj->full_hp = full_hp;
    pDerivedObj->now_hp = now_hp;
    pDerivedObj->color = al_map_rgb(255 ,30, 70);
    /*pDerivedObj->hitbox = New_Rectangle(pDerivedObj->x,
                                     pDerivedObj->y,
                                     pDerivedObj->x+pDerivedObj->full_length*pDerivedObj->now_hp/pDerivedObj->full_hp,
                                    pDerivedObj->y);*/
    pDerivedObj->font = al_create_builtin_font();
    // setting derived object function
    pObj->pDerivedObj = pDerivedObj;
    pObj->Update = Hpbar_update;
    pObj->Interact = Hpbar_interact;
    pObj->Draw = Hpbar_draw;
    pObj->Destroy = Hpbar_destroy;


    return pObj;
}
void Hpbar_update(Elements *self)
{
    Hpbar *hp = self->pDerivedObj;
    Elements *chara = get_susu();
    hp->now_hp = ((Damageable *)chara->pDerivedObj)->hp;
}
void _Hpbar_update_position(Elements *self, int dx, int dy)
{
    /*Hpbar *Obj = ((Hpbar *)(self->pDerivedObj));
    Obj->x += dx;
    Obj->y += dy;
    Shape *hitbox = Obj->hitbox;
    hitbox->update_center_x(hitbox, dx);
    hitbox->update_center_y(hitbox, dy);*/
}
void Hpbar_interact(Elements *self)
{
    /*for (int j = 0; j < self->inter_len; j++)
    {
        Hpbar *Obj =((Hpbar *)(self->pDerivedObj));
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
    }*/
}
void _Hpbar_interact_Floor(Elements *self, Elements *tar)
{
    /*Hpbar *Obj = ((Hpbar *)(self->pDerivedObj));
    if (Obj->x < 0 - Obj->width)
        self->dele = true;
    else if (Obj->x > WIDTH + Obj->width)
        self->dele = true;*/
}
void _Hpbar_interact_Tree(Elements *self, Elements *tar)
{
    /*Hpbar *Obj = ((Hpbar *)(self->pDerivedObj));
    Tree *tree = ((Tree *)(tar->pDerivedObj));
    if (tree->hitbox->overlap(tree->hitbox, Obj->hitbox))
    {
        self->dele = true;
    }*/
}
void Hpbar_draw(Elements *self)
{
    Hpbar *Obj = ((Hpbar *)(self->pDerivedObj));
    al_draw_rectangle(Obj->x,Obj->y,Obj->x+Obj->full_length,Obj->y+20,al_map_rgb(255 ,255, 255),2);
    //al_draw_line(Obj->x,Obj->y+10,Obj->x+Obj->full_length*Obj->now_hp/Obj->full_hp,Obj->y+10,Obj->color,20);
    al_draw_filled_rectangle(Obj->x,Obj->y,Obj->x+Obj->full_length,Obj->y+20,al_map_rgb(215 ,200, 200));
    al_draw_filled_rectangle(Obj->x,Obj->y,Obj->x+Obj->full_length*Obj->now_hp/Obj->full_hp,Obj->y+20,Obj->color);
    //ALLEGRO_FONT *font = al_create_builtin_font();
    ALLEGRO_TRANSFORM old;
    al_copy_transform(&old, al_get_current_transform());

    ALLEGRO_TRANSFORM tr;
    al_identity_transform(&tr);
    al_scale_transform(&tr, 2, 2);
    al_use_transform(&tr);

    al_draw_textf(Obj->font,al_map_rgb(255 ,255, 255),(Obj->x+Obj->full_length+5)/2,(Obj->y+2)/2,ALLEGRO_ALIGN_LEFT," %d/%d ",Obj->now_hp,Obj->full_hp);

    al_use_transform(&old);   // 還原
    
    //self->dele = true;
    
}
void Hpbar_destroy(Elements *self)
{
    Hpbar *Obj = ((Hpbar *)(self->pDerivedObj));
    //free(Obj->hitbox);
    al_destroy_font(Obj->font);
    free(Obj);
    free(self);
}



