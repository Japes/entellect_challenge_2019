#ifndef WORM_H
#define WORM_H

#include "Position.hpp"
#include "Weapon.hpp"
#include "GameConfig.hpp"

class GameState; //forward declaration

//current state of a worm

struct Worm
{
    GameState* state;
    unsigned id;
    int health;
    Position position;
    Position previous_position; //for pushback logic
    Weapon weapon;
    int diggingRange;
    int movementRange;

    Worm(GameState* _state);

    bool IsDead();
    void TakeDamage(int dmgAmount);

    bool operator==(const Worm &other) const;

};

#endif
