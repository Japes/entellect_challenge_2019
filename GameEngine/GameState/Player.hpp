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
    int command_score;
    int health;
    int currentWormId; //1-indexed
    int consecutiveDoNothingCount;
    std::vector<Worm> worms;

    Player(GameState* _state);
    Player(const Player& other);

    Worm* GetCurrentWorm();
    Worm* GetWormById(int index);
    void UpdateCurrentWorm();
    void RecalculateHealth();
    int GetAverageWormHealth();
    int GetScore();

    bool operator==(const Player &other) const;

};

#endif
