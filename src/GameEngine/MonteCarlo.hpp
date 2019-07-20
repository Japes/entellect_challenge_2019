#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

#include <memory>
#include "Commands/Command.hpp"

class MonteCarlo
{
	public:

    struct MCNode
    {
        MCNode(std::shared_ptr<Command> cmd) : 
            command{cmd},
            w{0},
            n{0},
            score{0},
            UCT{0}
        {};

        std::shared_ptr<Command> command;
        float w;
        float n;
        float score;
        float UCT;
    };

    MonteCarlo(const std::vector<std::shared_ptr<Command>>& cmds, float c);

    std::shared_ptr<MCNode> NextNode();
    void UpdateNumSamples();
    std::shared_ptr<Command> GetBestMove();
    void PrintState();

    private:
    int _N;
    float _c;
    std::vector<std::shared_ptr<MCNode>> _nodes;
};

std::ostream & operator << (std::ostream &out, const MonteCarlo::MCNode &move);

#endif
