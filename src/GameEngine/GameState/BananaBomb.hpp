#ifndef BANANA_BOMB_H
#define BANANA_BOMB_H

#include "GameConfig.hpp"

struct BananaBomb
{
    int damage;
    int range;
    int damageRadius;
                    
    BananaBomb()
    {
        damage = GameConfig::agentWorms.banana.damage;
        range = GameConfig::agentWorms.banana.range;
        damageRadius = GameConfig::agentWorms.banana.damageRadius;
    }
    
    bool operator==(const BananaBomb &other) const;
};

#endif
