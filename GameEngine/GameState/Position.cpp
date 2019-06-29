#include "Position.hpp"
#include "GameConfig.hpp"
#include <algorithm>
#include <iostream>

Position::Position() : x{0}, y{0}
{
}

Position::Position(int x, int y)
{
    this->x = x;
    this->y = y;
}

int Position::MaximumDimension(Position other) 
{
    return std::max(std::abs(x - other.x), std::abs(y - other.y));
}

//number of steps it would take to get to other
int Position::MovementDistanceTo(Position other)
{
   return MaximumDimension(other);
}

//range to target as defined in rules
int Position::ShootDistanceTo(Position other)
{
    auto xdist = other.x - x;
    auto ydist = other.y - y;
    return static_cast<int>(std::sqrt((xdist*xdist) + (ydist*ydist)));
}

bool Position::IsOnMap() const
{
    return x >= 0 && y >= 0 && x < GameConfig::mapSize && y < GameConfig::mapSize;
}

//to help with debugging...
std::ostream & operator << (std::ostream &out, const Position &pos)
{
    out << "[" << pos.x << ", " << pos.y << "]";
    return out;
}
