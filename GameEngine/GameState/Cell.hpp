#ifndef CELL_H
#define CELL_H

#include "Player.hpp"
#include "Powerup.hpp"
#include <stdint.h>

enum class CellType : uint8_t {
    AIR,
    DIRT,
    DEEP_SPACE
};

struct Cell
{
    CellType type;
    Player * player; //can be null
    PowerUp * powerup; //can be null

    Cell()
    {
        type = CellType::AIR;
        player = nullptr;
        powerup = nullptr;
    }
};

#endif
