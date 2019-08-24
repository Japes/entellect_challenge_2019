#include "MCNode.hpp"
#include <cmath>

MCNode::MCNode(std::shared_ptr<Command> cmd) : 
    _command{cmd},
    _w{0},
    _n{0},
    _UCT{0}
{}

void MCNode::UpdateUCT(int N, float c)
{
    if(_n == 0) {
        _UCT = std::numeric_limits<decltype(_UCT)>::max();
    } else {
        _UCT = (_w / _n) + c*std::sqrt(std::log(N)/_n );
    }
}

void MCNode::AddPlaythroughResult(float w)
{
    _w += w;
    ++_n;
}

float MCNode::GetUCT() const
{
    return _UCT;
}

float MCNode::GetN() const
{
    return _n;
}

std::shared_ptr<Command> MCNode::GetCommand() const
{
    return _command;
}
