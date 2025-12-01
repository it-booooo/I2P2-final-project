#include "element.h"
#include <cstdlib>
#include <cstring>

DataCenter *DC = DataCenter::get_instance();

Elements::Elements(int label_value)
{
    label = label_value;
    id = 0;
    entity = nullptr;
    inter_len = 0;
    dele = false;
    Draw = nullptr;
    Update = nullptr;
    Interact = nullptr;
    Destroy = nullptr;
    std::memset(inter_obj, 0, sizeof(inter_obj));
}

Elements::~Elements() = default;

/*
   [Element function]
*/
Elements *New_Elements(int label)
{
    Elements *pObj;
    pObj = new Elements(label);
    return pObj;
}
