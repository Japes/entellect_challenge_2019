#ifndef PLAYER_H
#define PLAYER_H

#include "Worm.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

class GameState; //forward declaration
class Command; //forward declaration

//current state of a player
struct Player
{
    GameState* state;
    unsigned id;
    int command_score;
    int health;
    int currentWormId; //1-indexed
    int consecutiveDoNothingCount;
    int remainingWormSelections;
    std::array<Worm, 3> worms;
    std::shared_ptr<Command> previousCommand;

    Player(GameState* _state);
    Player(const Player& other);

    void SetId(int _id);

    Worm* GetCurrentWorm();
    Worm* GetWormById(int index);
    void UpdateCurrentWorm();
    void RecalculateHealth();
    int GetAverageWormHealth();
    int GetScore();

    bool operator==(const Player &other) const;

private:
    std::vector<Worm*> _livingWorms; //for performance reasons

};

#endif
