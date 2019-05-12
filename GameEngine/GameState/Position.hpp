#ifndef POSITION_H
#define POSITION_H

#include <algorithm>
#include <iostream>

struct Position
{
    int x;
    int y;

    Position();
    Position(int x, int y);
    int MaximumDimension(Position other);
    int MovementDistanceTo(Position other);

    //fun shootingDistance(other: Point): Double = floor(euclideanDistance(other))
    //fun manhattanDistance(other: Point) = abs(x - other.x) + abs(y - other.y)
    //fun euclideanDistance(other: Point) = sqrt((x - other.x).pow(2) + (y - other.y).pow(2))
    //fun euclideanDistance(other: Pair<Double, Double>) = sqrt((x - other.first).pow(2) + (y - other.second).pow(2))

};

std::ostream & operator << (std::ostream &out, const Position &pos);

#endif
