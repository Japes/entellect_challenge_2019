#ifndef POSITION_H
#define POSITION_H

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
};

#endif
