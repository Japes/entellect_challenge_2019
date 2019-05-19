#ifndef PLAYER_H
#define PLAYER_H

#include "Worm.hpp"

#include <vector>
#include <algorithm>
#include <stdexcept>

class GameState; //forward declaration

//current state of a player
struct Player
{
    GameState* state;
    unsigned id;
    int score;
    int health;
    int currentWormId; //1-indexed
    int consecutiveDoNothingCount;
    std::vector<Worm> worms;

    Player(GameState* _state);

    Worm* GetCurrentWorm();
    Worm* GetWormByIndex(int index);
    void UpdateCurrentWorm();
    void RecalculateHealth();
};

#endif
