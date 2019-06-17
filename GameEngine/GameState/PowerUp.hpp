#ifndef POWERUP_H
#define POWERUP_H

struct PowerUp
{
    //"type": "HEALTH_PACK", //TODO add enum when we have more types
    int value;

    PowerUp() {
        value = GameConfig::healthPackHp;
    }

    void ApplyTo(Worm* worm)
    {
        worm->health += value;
    }

    bool operator==(const PowerUp &other) const
    {
        return value == other.value;
    }

};

#endif
