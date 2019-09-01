#include "PlayersMonteCarlo.hpp"
#include <cmath>

PlayersMonteCarlo::PlayersMonteCarlo(const std::vector<std::shared_ptr<Command>>& cmds, float c) : _N{0}, _c{c}
{
    for(auto const& cmd: cmds) {
        _moves.push_back(std::make_shared<MCMove>(cmd));
    }
}

PlayersMonteCarlo::PlayersMonteCarlo(std::vector<std::shared_ptr<MCMove>> moves, float c) : _N{0}, _c{c}, _moves{moves}
{
}

std::shared_ptr<MCMove> PlayersMonteCarlo::NextMove()
{
    for(auto & move: _moves) {
        move->UpdateUCT(_N, _c);
    }

    auto next_node = std::max_element(std::begin(_moves), std::end(_moves), 
            [] (std::shared_ptr<MCMove> const lhs, std::shared_ptr<MCMove> const rhs) -> bool { 
                return lhs->GetUCT() < rhs->GetUCT(); });

    return (*next_node);
}

void PlayersMonteCarlo::UpdateNumSamples()
{
    ++_N;
}

std::shared_ptr<Command> PlayersMonteCarlo::GetBestMove()
{
    auto it = std::max_element(std::begin(_moves), std::end(_moves),
            [] (std::shared_ptr<MCMove> const lhs, std::shared_ptr<MCMove> const rhs) -> bool { 
                return lhs->GetN() < rhs->GetN(); });
    return (*it)->GetCommand();
}

void PlayersMonteCarlo::PrintState()
{
    std::cerr << "N: " << _N << std::endl; //print this first so it comes up in PlayerCommand.txt
    std::cerr << "MC results: " << std::endl;
    for(auto const & move : _moves) {
        std::cerr << *move << std::endl;
    }
    std::cerr << " best move is " << GetBestMove()->GetCommandString() << std::endl;
}
