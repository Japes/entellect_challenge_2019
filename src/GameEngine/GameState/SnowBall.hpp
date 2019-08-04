#ifndef SNOW_BALL_H
#define SNOW_BALL_H

#include "GameConfig.hpp"

struct SnowBall
{
    int freezeDuration;
    int range;
    int freezeRadius;
                    
    SnowBall()
    {
        freezeDuration = GameConfig::technologistWorms.snowball.freezeDuration;
        range = GameConfig::technologistWorms.snowball.range;
        freezeRadius = GameConfig::technologistWorms.snowball.freezeRadius;
    }
    
    bool operator==(const SnowBall &other) const;
};

#endif
