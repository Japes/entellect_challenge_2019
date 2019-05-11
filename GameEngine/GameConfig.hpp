#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#define MAP_SIZE ((int)33)

struct WeaponConfig
{
    const int damage = 8;
    const int range = 4;
};

struct CommandoWorms
{
    const int count = 3;
    const int initialHp = 150;
    const int movementRange = 1;
    const int diggingRange = 1;
    const WeaponConfig weapon;
};

struct Scores
{
    const int missedAttack =  2;
    const int attack =  20;
    const int killShot =  40;
    const int friendlyFire =  -20;
    const int dig =  7;
    const int move =  5;
    const int powerup =  20;
    const int doNothing =  0;
    const int invalidCommand =  -4;
};


struct GameConfig
{
    static const int maxRounds = 400;
    static const int maxDoNothings = 12;
    static const CommandoWorms commandoWorms;
    static const int pushbackDamage =  20;
    static const int mapSize = MAP_SIZE;
    static const int healthPackHp =  10;
    static const int totalHealthPacks =  2;
    static const Scores scores;
};

#endif
