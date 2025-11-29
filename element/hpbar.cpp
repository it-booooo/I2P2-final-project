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
    Hpbar *entity = (Hpbar *)malloc(sizeof(Hpbar));
    Elements *pObj = New_Elements(label);
    // setting derived object member
    entity->x = 10;
    entity->y = 1300;
    entity->full_length=400;
    entity->full_hp = full_hp;
    entity->now_hp = now_hp;
    entity->color = al_map_rgb(255 ,30, 70);
    /*entity->hitbox = New_Rectangle(entity->x,
                                     entity->y,
                                     entity->x+entity->full_length*entity->now_hp/entity->full_hp,
                                    entity->y);*/
    entity->font = al_create_builtin_font();
    // setting derived object function
    pObj->entity = entity;
    pObj->Update = Hpbar_update;
    pObj->Interact = Hpbar_interact;
    pObj->Draw = Hpbar_draw;
    pObj->Destroy = Hpbar_destroy;


    return pObj;
}
void Hpbar_update(Elements *self)
{
    Hpbar *hp = static_cast<Hpbar *>(self->entity);
    Elements *chara = get_susu();
    hp->now_hp = ((Damageable *)chara->entity)->hp;
}
void _Hpbar_update_position(Elements *self, int dx, int dy)
{
    /*Hpbar *Obj = ((Hpbar *)(self->entity));
    Obj->x += dx;
    Obj->y += dy;
    Shape *hitbox = Obj->hitbox;
    hitbox->update_center_x(hitbox->center_x() + dx);
    hitbox->update_center_y(hitbox->center_y() + dy);*/
}
void Hpbar_interact(Elements *self)
{
    /*for (int j = 0; j < self->inter_len; j++)
    {
        Hpbar *Obj =((Hpbar *)(self->entity));
        ElementVec labelEle = _Get_label_elements(scene, Character_L);
        Character *Obj1 = (Character *)(labelEle.arr[0]->entity);
        labelEle = _Get_label_elements(scene, Tree_L);
        Tree *Obj2 =(Tree *)(labelEle.arr[0]->entity);


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
    /*Hpbar *Obj = ((Hpbar *)(self->entity));
    if (Obj->x < 0 - Obj->width)
        self->dele = true;
    else if (Obj->x > WIDTH + Obj->width)
        self->dele = true;*/
}
void _Hpbar_interact_Tree(Elements *self, Elements *tar)
{
    /*Hpbar *Obj = ((Hpbar *)(self->entity));
    Tree *tree = ((Tree *)(tar->entity));
    if (tree->hitbox->overlap(tree->hitbox, Obj->hitbox))
    {
        self->dele = true;
    }*/
}
void Hpbar_draw(Elements *self)
{
    Hpbar *Obj = ((Hpbar *)(self->entity));
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
    Hpbar *Obj = ((Hpbar *)(self->entity));
    //free(Obj->hitbox);
    al_destroy_font(Obj->font);
    free(Obj);
    free(self);
}



