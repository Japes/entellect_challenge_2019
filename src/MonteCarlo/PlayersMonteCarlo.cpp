#include "PlayersMonteCarlo.hpp"
#include <cmath>

PlayersMonteCarlo::PlayersMonteCarlo(const std::vector<std::shared_ptr<Command>>& cmds, float c) : _N{0}, _c{c}
{
    _moves.reserve(cmds.size());

    for(auto const& cmd: cmds) {
        _moves.emplace_back(MCMove(cmd));
    }
}

//for debug
std::vector<MCMove*> PlayersMonteCarlo::GetNodes()
{
    std::vector<MCMove*> ret;
    for(auto & move: _moves) {
        ret.push_back(&move);
    }
    return ret;
}

MCMove* PlayersMonteCarlo::NextMove()
{
    for(auto & move: _moves) {
        move.UpdateUCT(_N, _c);
    }

    //TODO should resolve ties with a random pick here
    auto next_node = std::max_element(std::begin(_moves), std::end(_moves), 
            [] (MCMove const lhs, MCMove const rhs) -> bool { 
                return lhs.GetUCT() < rhs.GetUCT(); });

    if(next_node == std::end(_moves)) {
         std::cerr << "(" << __FUNCTION__ << ") I have no moves!" << std::endl;
         return nullptr;
    }

    return &(*next_node);
}

void PlayersMonteCarlo::UpdateNumSamples()
{
    ++_N;
}

std::shared_ptr<Command> PlayersMonteCarlo::GetBestMove()
{
    auto it = std::max_element(std::begin(_moves), std::end(_moves),
            [] (MCMove const lhs, MCMove const rhs) -> bool { 
                return lhs.GetN() < rhs.GetN(); });
    return it->GetCommand();
}

void PlayersMonteCarlo::PrintState()
{
    std::cerr << "N: " << _N << std::endl; //print this first so it comes up in PlayerCommand.txt
    std::cerr << "MC results: " << std::endl;
    for(auto const & move : _moves) {
        std::cerr << move << std::endl;
    }
    std::cerr << " best move is " << GetBestMove()->GetCommandString() << std::endl;
}
