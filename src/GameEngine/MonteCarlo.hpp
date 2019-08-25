#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

#include <memory>
#include "Commands/Command.hpp"
#include "MCMove.hpp"

class MonteCarlo
{
	public:

    MonteCarlo(const std::vector<std::shared_ptr<Command>>& cmds, float c);
    MonteCarlo(std::vector<std::shared_ptr<MCMove>> nodes, float c);

    std::shared_ptr<MCMove> NextNode();
    void UpdateNumSamples();
    std::shared_ptr<Command> GetBestMove();
    void PrintState();

    private:
    int _N;
    float _c;
    std::vector<std::shared_ptr<MCMove>> _nodes;
};


#endif
