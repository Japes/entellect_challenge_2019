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
    uint8_t id;
    int16_t command_score;
    int16_t health;
    int8_t currentWormId; //1-indexed
    int8_t consecutiveDoNothingCount;
    int8_t remainingWormSelections;
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
