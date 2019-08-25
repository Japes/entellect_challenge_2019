#include "MCMove.hpp"
#include <cmath>

MCMove::MCMove(std::shared_ptr<Command> cmd) : 
    _command{cmd},
    _w{0},
    _n{0},
    _UCT{0}
{}

void MCMove::UpdateUCT(int N, float c)
{
    if(_n == 0) {
        _UCT = std::numeric_limits<decltype(_UCT)>::max();
    } else {
        _UCT = (_w / _n) + c*std::sqrt(std::log(N)/_n );
    }
}

void MCMove::AddPlaythroughResult(float w)
{
    _w += w;
    ++_n;
}

float MCMove::GetUCT() const
{
    return _UCT;
}

float MCMove::GetN() const
{
    return _n;
}

std::shared_ptr<Command> MCMove::GetCommand() const
{
    return _command;
}
