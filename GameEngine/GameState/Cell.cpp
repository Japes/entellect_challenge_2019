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
