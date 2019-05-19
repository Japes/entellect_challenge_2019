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
    int consecutiveDoNothingCount;
    std::vector<Worm> worms;

    Player() :
        id{0},
        score{0},
        health{0},
        currentWormId{1},
        consecutiveDoNothingCount{0}
    {
        //not sure why the usual vector constructors aren't working here...
        worms.push_back(Worm());
        worms.push_back(Worm());
        worms.push_back(Worm());
        RecalculateHealth();
    }

    Worm* GetCurrentWorm()
    {
        unsigned wormIndex = currentWormId - 1;
        if(wormIndex >= worms.size()) {
            throw std::runtime_error("Problem getting next worm for player");
        }
        return &worms[wormIndex];
    }

    Worm* GetWormByIndex(int index)
    {
        return &worms[index - 1];
    }

    void UpdateCurrentWorm()
    {
        ++currentWormId;
        if(currentWormId > static_cast<int>(worms.size())) {
            currentWormId = 1;
        }
    }

    void RecalculateHealth()
    {
        health = 0;
        std::for_each(worms.begin(), worms.end(), [&](Worm w){ health += (w.health > 0) ? w.health : 0;});
    }
};

#endif
