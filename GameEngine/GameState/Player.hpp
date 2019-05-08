#ifndef PLAYER_H
#define PLAYER_H

#include "Worm.hpp"
#include <vector>

//current state of a player

struct Player
{
    unsigned id;
    int score;
    int health;
    int currentWormId;
    std::vector<Worm> worms;
};

#endif
