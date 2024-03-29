#ifndef POSITION_H
#define POSITION_H

#include <algorithm>
#include <iostream>

struct Position
{
    int16_t x;
    int16_t y;

    Position();
    Position(int x, int y);
    int MaximumDimension(const Position &other) const;
    int MovementDistanceTo(const Position &other) const;
    float EuclideanDistanceTo(const Position &other) const;
    int SquareDistanceTo(const Position &other) const;
    bool IsOnMap() const;
    bool BananaSnowballCanReach(const Position &other) const;
    void ClampToMap();
    Position Normalized() const;

    //fun shootingDistance(other: Point): Double = floor(euclideanDistance(other))
    //fun manhattanDistance(other: Point) = abs(x - other.x) + abs(y - other.y)
    //fun euclideanDistance(other: Point) = sqrt((x - other.x).pow(2) + (y - other.y).pow(2))
    //fun euclideanDistance(other: Pair<Double, Double>) = sqrt((x - other.first).pow(2) + (y - other.second).pow(2))

    Position operator+(const Position& other) const
    {
        return Position(x + other.x, y + other.y);
    }

    void operator+=(const Position& other)
    {
        x += other.x;
        y += other.y;
    }

    Position operator-(const Position& other) const
    {
        return Position(x - other.x, y - other.y);
    }

    bool operator==(const Position& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Position& other) const
    {
        return ! (*this == other);
    }

};

std::ostream & operator << (std::ostream &out, const Position &pos);

#endif
