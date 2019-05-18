#ifndef WEAPON_H
#define WEAPON_H

#include "GameConfig.hpp"

struct Weapon
{
    int damage;
    int range;
    int diagRange;

    Weapon()
    {
        damage = GameConfig::commandoWorms.weapon.damage;
        range = GameConfig::commandoWorms.weapon.range;
        diagRange = GameConfig::commandoWorms.weapon.diagRange;
    }
};

#endif
