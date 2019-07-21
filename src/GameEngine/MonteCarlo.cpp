#include "MonteCarlo.hpp"
#include <cmath>

MonteCarlo::MonteCarlo(const std::vector<std::shared_ptr<Command>>& cmds, float c) : _N{0}, _c{c}
{
    for(auto const& cmd: cmds) {
        _nodes.push_back(std::make_shared<MCNode>(cmd));
    }
}

std::shared_ptr<MonteCarlo::MCNode> MonteCarlo::NextNode()
{
    for(auto & node: _nodes) {
        if(node->n == 0) {
            node->UCT = std::numeric_limits<decltype(node->UCT)>::max();
        } else {
            node->UCT = (node->w / node->n) + _c*std::sqrt(std::log(_N)/node->n );
        }
    }

    auto next_node = std::max_element(std::begin(_nodes), std::end(_nodes), 
            [] (std::shared_ptr<MonteCarlo::MCNode> const lhs, std::shared_ptr<MonteCarlo::MCNode> const rhs) -> bool { 
                return lhs->UCT < rhs->UCT; });

    return (*next_node);
}

void MonteCarlo::UpdateNumSamples()
{
    ++_N;
}

std::shared_ptr<Command> MonteCarlo::GetBestMove()
{
    auto it = std::max_element(std::begin(_nodes), std::end(_nodes), 
        [] (std::shared_ptr<MonteCarlo::MCNode> const lhs, std::shared_ptr<MonteCarlo::MCNode> const rhs) -> bool { return (lhs->w/lhs->n) < (rhs->w/rhs->n); });

    return (*it)->command;
}

void MonteCarlo::PrintState()
{
    std::cerr << "N: " << _N << std::endl; //print this first so it comes up in PlayerCommand.txt
    std::cerr << "MC results: " << std::endl;
    for(auto const & move : _nodes) {
        std::cerr << *move << std::endl;
    }
    std::cerr << " best move is " << GetBestMove()->GetCommandString() << std::endl;
}

std::ostream & operator << (std::ostream &out, const MonteCarlo::MCNode &move)
{
    out << move.command->GetCommandString() << ", " <<  move.w << "/" << move.n << ", " << move.score << ", " << move.UCT;
    return out;
}