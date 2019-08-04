#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#define MAP_SIZE ((int)33)

struct WeaponConfig
{
    const int damage = 8;
    const int range = 4;    //range in a straight line //NOTE YOU NEED TO UPDATE DIAG RANGE IF YOU UPDATE THIS
    const int diagRange = 3; //diagonal range. //NOTE THIS MUST BE MANUALLY UPDATED EVERY TIME YOU UPDATE RANGE
};

struct BananaBombConfig
{
    const int damage = 20;
    const int range = 5;
    const int count = 3;
    const int damageRadius = 2;
};

struct SnowballConfig
{
    const int freezeDuration = 5;
    const int range = 5;
    const int count = 3;
    const int freezeRadius = 1;
};

struct TechnologistWorms
{
    const int count = 1;
    const int initialHp = 100;
    const int movementRange = 1;
    const int diggingRange = 1;
    const WeaponConfig weapon;
    const SnowballConfig snowball;
};

struct AgentWorms
{
    const int count = 1;
    const int initialHp = 100;
    const int movementRange = 1;
    const int diggingRange = 1;
    const WeaponConfig weapon;
    const BananaBombConfig banana;
};

struct CommandoWorms
{
    const int count = 1;
    const int initialHp = 150;
    const int movementRange = 1;
    const int diggingRange = 1;
    const WeaponConfig weapon;
};

struct Scores
{
    const int missedAttack = 2;
    const int killShot = 40;
    const int dig = 7;
    const int move = 5;
    const int powerup = 20;
    const int doNothing = 0;
    const int invalidCommand = -4;
    const int freeze = 17;
};

struct GameConfig
{
    static const int maxRounds = 400;
    static const int maxDoNothings = 12;
    static const int lavaDamage = 3;
    static const CommandoWorms commandoWorms;
    static const AgentWorms agentWorms;
    static const TechnologistWorms technologistWorms;
    static const int pushbackDamage =  20;
    static const int mapSize = MAP_SIZE;
    static const int healthPackHp =  10;
    static const int totalHealthPacks = 2;
    static const int wormSelectTokens = 5;
    static const Scores scores;
    static const float battleRoyaleStart;
    static const float battleRoyaleEnd;
};

#endif
