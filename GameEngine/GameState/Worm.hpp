#ifndef WORM_H
#define WORM_H

#include "Position.hpp"
#include "Weapon.hpp"

//current state of a worm

struct Worm
{
    unsigned id;
    int health;
    Position position;
    Weapon weapon;
    int diggingRange;
    int movementRange;
};

#endif
