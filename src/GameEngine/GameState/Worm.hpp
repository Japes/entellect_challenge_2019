#ifndef WORM_H
#define WORM_H

#include "Position.hpp"
#include "Weapon.hpp"
#include "BananaBomb.hpp"
#include "SnowBall.hpp"
#include "GameConfig.hpp"

class GameState; //forward declaration

//current state of a worm

struct Worm
{
    enum class Proffession : uint8_t {
        COMMANDO,
        AGENT, //the guy with the bombs
        TECHNOLOGIST //the freeze guy
    };

    GameState* state;
    unsigned id;
    int health;
    Position position;
    Position previous_position; //for pushback logic
    Weapon weapon;
    BananaBomb banana_bomb;
    SnowBall snowball;
    unsigned banana_bomb_count;
    unsigned snowball_count;
    int diggingRange;
    int movementRange;
    Proffession proffession;
    bool movedThisRound;
    bool diedByLavaThisRound;
    int roundsUntilUnfrozen;


    Worm(GameState* _state, Worm::Proffession _proffession = Worm::Proffession::COMMANDO);

    void SetProffession(Worm::Proffession _proffession);

    bool IsDead() const;
    bool IsFrozen() const;
    void TakeDamage(int dmgAmount);

    bool operator==(const Worm &other) const;

};

#endif
