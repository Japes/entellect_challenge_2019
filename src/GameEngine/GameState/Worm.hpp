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
    uint8_t playerId;
    uint8_t id;
    int16_t health;
    Position position;
    Position previous_position; //for pushback logic
    Weapon weapon;
    BananaBomb banana_bomb;
    SnowBall snowball;
    uint8_t banana_bomb_count;
    uint8_t snowball_count;
    int8_t diggingRange;
    int8_t movementRange;
    Proffession proffession;
    bool movedThisRound;
    bool diedByLavaThisRound;
    bool frozenThisRound;
    int8_t roundsUntilUnfrozen;
    std::vector<Worm*> lastAttackedBy;

    Worm(GameState* _state, Worm::Proffession _proffession = Worm::Proffession::COMMANDO);

    void SetProffession(Worm::Proffession _proffession);

    bool IsDead() const;
    bool IsFrozen() const;
    void TakeDamage(int dmgAmount, Worm* attacker = nullptr);

    bool operator==(const Worm &other) const;

};

#endif
