#include "Position.hpp"
#include "GameConfig.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

Position::Position() : x{0}, y{0}
{
}

Position::Position(int x, int y)
{
    this->x = x;
    this->y = y;
}

int Position::MaximumDimension(const Position &other) const
{
    return std::max(std::abs(x - other.x), std::abs(y - other.y));
}

//number of steps it would take to get to other
int Position::MovementDistanceTo(const Position &other) const
{
   return MaximumDimension(other);
}

//range to target as defined in rules
float Position::EuclideanDistanceTo(const Position &other) const
{
    float xdist = other.x - x;
    float ydist = other.y - y;
    return std::sqrt((xdist*xdist) + (ydist*ydist));
}

int Position::SquareDistanceTo(const Position &other) const
{
    auto xdist = other.x - x;
    auto ydist = other.y - y;
    return static_cast<int>((xdist*xdist) + (ydist*ydist));
}

//adjusts position so that it is on the map
void Position::ClampToMap()
{
    if(x < 0)               { x = 0; }
    if(x > (MAP_SIZE - 1))  { x = (MAP_SIZE - 1); }
    if(y < 0)               { y = 0; }
    if(y > (MAP_SIZE - 1))  { y = (MAP_SIZE - 1); }
}

bool Position::IsOnMap() const
{
    return x >= 0 && y >= 0 && x < GameConfig::mapSize && y < GameConfig::mapSize;
}

bool Position::BananaSnowballCanReach(const Position &other) const
{
       /*xdist + ydist in bananas radius:
    0   1   2   3   4   5   6   7   8   9   10
    0   .   .   .   .   .   .   .   .   .   .
    1   .   5   6   7   8   .   .   .   .   .
    2   .   4   5   6   7   8   .   .   .   .
    3   .   3   4   5   6   7   8   .   .   .
    4   .   2   3   4   5   6   7   .   .   .
    5   .   1   2   3   4   5   6   .   .   .
    6   .   W   1   2   3   4   5   .   .   .
    7   .   .   .   .   .   .   .   .   .   .    
    */

    auto distX = std::abs(x - other.x);
    auto distY = std::abs(y - other.y);
    bool isInTheCorner = (distX + distY) > 8;
    return !isInTheCorner && (MaximumDimension(other) <= GameConfig::agentWorms.banana.range); //BIG ASSUMPTION HERE: BANANA IS THE SAME AS SNOWBALL
}

Position Position::Normalized() const
{
    Position ret(0,0);
    if(x > 0) {
        ret.x = 1;
    } else if (x < 0) {
        ret.x = -1;
    }

    if(y > 0) {
        ret.y = 1;
    } else if (y < 0) {
        ret.y = -1;
    }

    return ret;
}

//to help with debugging...
std::ostream & operator << (std::ostream &out, const Position &pos)
{
    out << "[" << pos.x << ", " << pos.y << "]";
    return out;
}
