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

CellType strToCellType(std::string str);

struct Cell
{
    CellType type;
    Worm * worm{nullptr}; //can be null
    PowerUp * powerup{nullptr}; //can be null

    Cell()
    {
        type = CellType::AIR;
        worm = nullptr;
        powerup = nullptr;
    }
};

#endif
