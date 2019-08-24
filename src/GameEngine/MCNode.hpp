#ifndef MCNODE_H
#define MCNODE_H

#include <memory>
#include "Commands/Command.hpp"

class MCNode
{
    public:
    MCNode(std::shared_ptr<Command> cmd);

    void UpdateUCT(int N, float c);
    void AddPlaythroughResult(float w);

    std::shared_ptr<Command> GetCommand() const;
    float GetUCT() const;
    float GetN() const;

    friend std::ostream & operator << (std::ostream &out, const MCNode &move)
    {
        out << move.GetCommand()->GetCommandString() << ", " << move._w << "/" << move._n << ", UCT: " << move.GetUCT();
        return out;
    }

    private:
    std::shared_ptr<Command> _command;
    float _w;
    float _n;
    float _UCT;
};


#endif
