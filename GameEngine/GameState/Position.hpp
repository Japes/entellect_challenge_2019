#ifndef POSITION_H
#define POSITION_H

#include <algorithm>

struct Position
{
    int x;
    int y;

    Position() :
        x{0},
        y{0}
    {
    }

    Position(int x, int y)
    {
        this->x = x;
        this->y = y;
    }

    int MaximumDimension(Position other) 
    {
        return std::max(std::abs(x - other.x), abs(y - other.y));
    }

    int MovementDistanceTo(Position other)
    {
       return MaximumDimension(other);
    };
        

    //fun shootingDistance(other: Point): Double = floor(euclideanDistance(other))
    //fun manhattanDistance(other: Point) = abs(x - other.x) + abs(y - other.y)
    //fun euclideanDistance(other: Point) = sqrt((x - other.x).pow(2) + (y - other.y).pow(2))
    //fun euclideanDistance(other: Pair<Double, Double>) = sqrt((x - other.first).pow(2) + (y - other.second).pow(2))
};

#endif
