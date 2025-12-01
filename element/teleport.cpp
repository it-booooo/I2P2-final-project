#include "teleport.h"
#include "../scene/gamescene.h" // for element label
#include "../scene/sceneManager.h" // for scene variable
#include "charater.h"
#include "../data/DataCenter.h"
/*
   [teleport function]
*/
Elements *New_Teleport(int label)
{
    Teleport *entity = (Teleport *)malloc(sizeof(Teleport));
    Elements *pObj = New_Elements(label);
    Elements &wrapper = *pObj;
    Teleport &obj = *entity;
    // setting derived object member
    obj.img = al_load_bitmap("assets/image/teleport.png");
    obj.width = al_get_bitmap_width(obj.img);
    obj.height = al_get_bitmap_height(obj.img);
    obj.x = DataCenter::WIDTH - obj.width;
    obj.y = DataCenter::HEIGHT - obj.height;
    obj.activate = false;
    // setting the interact object
    wrapper.inter_obj[wrapper.inter_len++] = Character_L;
    // setting derived object function
    wrapper.entity = entity;
    wrapper.Update = Teleport_update;
    wrapper.Interact = Teleport_interact;
    wrapper.Draw = Teleport_draw;
    wrapper.Destroy = Teleport_destory;
    return pObj;
}
void Teleport_update(Elements *self)
{
    Elements &wrapper = *self;
    Teleport &Obj = *reinterpret_cast<Teleport *>(wrapper.entity);
    DataCenter *DC = DataCenter::get_instance();
    if (DC->key_state[ALLEGRO_KEY_W])
    {
        Obj.activate = true;
    }
    else
    {
        Obj.activate = false;
    }
}
void Teleport_interact(Elements *self)
{
    ElementVec labelEle = _Get_label_elements(scene, Character_L);
    for (int i = 0; i < labelEle.len; i++)
    {
        _Teleport_interact_Character(self, labelEle.arr[i]);
    }
}
void _Teleport_interact_Character(Elements *self, Elements *tar)
{
    if (!tar) return;
    Elements &tar_wrapper = *tar;
    Character *chara_ptr = (Character *)(tar_wrapper.entity);
    if (!chara_ptr) return;
    Character &chara = *chara_ptr;
    Elements &wrapper = *self;
    Teleport &Obj = *static_cast<Teleport *>(wrapper.entity);
    if (chara.x >= Obj.x &&
        chara.x <= Obj.x + Obj.width &&
        Obj.activate)
    {
        _Character_update_position(tar, 0 - chara.x, 0);
    }
}
void Teleport_draw(Elements *self)
{
    Elements &wrapper = *self;
    Teleport &Obj = *static_cast<Teleport *>(wrapper.entity);
    al_draw_bitmap(Obj.img, Obj.x, Obj.y, 0);
}
void Teleport_destory(Elements *self)
{
    Elements &wrapper = *self;
    Teleport &Obj = *static_cast<Teleport *>(wrapper.entity);
    al_destroy_bitmap(Obj.img);
    free(wrapper.entity);
    free(self);
}
