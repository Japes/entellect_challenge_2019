#ifndef WORM_H
#define WORM_H

#include "Position.hpp"
#include "Weapon.hpp"
#include "GameConfig.hpp"

//current state of a worm

struct Worm
{
    unsigned id;
    int health;
    Position position;
    Weapon weapon;
    int diggingRange;
    int movementRange;

    Worm() : id{0}
    {
        health = GameConfig::commandoWorms.initialHp;
        diggingRange = GameConfig::commandoWorms.diggingRange;
        movementRange = GameConfig::commandoWorms.movementRange;
    }
};

#endif
