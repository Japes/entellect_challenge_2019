#ifndef PLAYERS_MONTE_CARLO_H
#define PLAYERS_MONTE_CARLO_H

#include <memory>
#include "Commands/Command.hpp"
#include "MCMove.hpp"

class PlayersMonteCarlo
{
	public:

    PlayersMonteCarlo(const std::vector<std::shared_ptr<Command>>& cmds, float c);
    
    std::vector<MCMove*> GetNodes();

    MCMove* NextMove();
    void UpdateNumSamples();
    std::shared_ptr<Command> GetBestMove();
    void PrintState();

    private:
    int _N;
    float _c;
    std::vector<MCMove> _moves;
};


#endif
