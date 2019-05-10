#ifndef WEAPON_H
#define WEAPON_H

#include "GameConfig.hpp"

struct Weapon
{
    int damage;
    int range;

    Weapon()
    {
        damage = GameConfig::commandoWorms.weapon.damage;
        range = GameConfig::commandoWorms.weapon.range;
    }
};

#endif
