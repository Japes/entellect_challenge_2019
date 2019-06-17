#include "Cell.hpp"

CellType Cell::strToCellType(std::string str)
{
    if(str == "DEEP_SPACE") {
        return CellType::DEEP_SPACE;
    } else if(str == "AIR") {
        return CellType::AIR;
    }else if(str == "DIRT") {
        return CellType::DIRT;
    }
    return CellType::AIR;
}

bool Cell::operator==(const Cell &other) const
{
    return type == other.type &&
            (worm == other.worm || *worm == *other.worm) && 
            (powerup == other.powerup || *powerup == *other.powerup); 
}
