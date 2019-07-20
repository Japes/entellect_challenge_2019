#ifndef WORM_H
#define WORM_H

#include "Position.hpp"
#include "Weapon.hpp"
#include "BananaBomb.hpp"
#include "GameConfig.hpp"

class GameState; //forward declaration

//current state of a worm

struct Worm
{
    enum class Proffession : uint8_t {
        COMMANDO,
        AGENT //the guy with the bombs
    };

    GameState* state;
    unsigned id;
    int health;
    Position position;
    Position previous_position; //for pushback logic
    Weapon weapon;
    BananaBomb banana_bomb;
    unsigned banana_bomb_count;
    int diggingRange;
    int movementRange;
    Proffession proffession;
    bool movedThisRound;

    Worm(GameState* _state, bool agent = false);

    bool IsDead() const;
    void TakeDamage(int dmgAmount);

    bool operator==(const Worm &other) const;

};

#endif
