#ifndef CELL_H
#define CELL_H

#include "Player.hpp"
#include "PowerUp.hpp"
#include <stdint.h>

enum class CellType : uint8_t {
    AIR,
    DIRT,
    DEEP_SPACE
};

inline bool IsBlocking(CellType t)
{
    return t == CellType::DIRT || t == CellType::DEEP_SPACE;
}

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

    static CellType strToCellType(std::string str);

    bool operator==(const Cell &other) const;

};

#endif
