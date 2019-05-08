#ifndef PLAYER_H
#define PLAYER_H

#include "Worm.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>

//current state of a player

struct Player
{
    unsigned id;
    int score;
    int health;
    int currentWormId; //1-indexed
    std::vector<Worm> worms;

    Worm GetCurrentWorm()
    {
        unsigned wormIndex = currentWormId - 1;
        if(wormIndex >= worms.size()) {
            throw std::runtime_error("Problem getting next worm for player");
        }
        return worms[wormIndex];
    }

    void UpdateCurrentWorm()
    {
        ++currentWormId;
        if(currentWormId > static_cast<int>(worms.size())) {
            currentWormId = 1;
        }
    }
};

#endif
