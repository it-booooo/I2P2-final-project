#ifndef DAMAGEABLE_H
#define DAMAGEABLE_H
#include "../shapes/Shape.h"
#include "element.h"

class Damageable
{
public:
    int side; // 0 for susu, 1 for moster
    int hp;
    int full_hp;
    Shape *hitbox;
};

void DealDamageIfPossible(Elements *target, int damage);
#endif /* DAMAGEABLE_H */
