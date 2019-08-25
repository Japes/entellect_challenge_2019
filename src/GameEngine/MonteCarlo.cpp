#include "MonteCarlo.hpp"
#include <cmath>

MonteCarlo::MonteCarlo(const std::vector<std::shared_ptr<Command>>& cmds, float c) : _N{0}, _c{c}
{
    for(auto const& cmd: cmds) {
        _nodes.push_back(std::make_shared<MCMove>(cmd));
    }
}

MonteCarlo::MonteCarlo(std::vector<std::shared_ptr<MCMove>> nodes, float c) : _N{0}, _c{c}, _nodes{nodes}
{
}

std::shared_ptr<MCMove> MonteCarlo::NextNode()
{
    for(auto & node: _nodes) {
        node->UpdateUCT(_N, _c);
    }

    auto next_node = std::max_element(std::begin(_nodes), std::end(_nodes), 
            [] (std::shared_ptr<MCMove> const lhs, std::shared_ptr<MCMove> const rhs) -> bool { 
                return lhs->GetUCT() < rhs->GetUCT(); });

    return (*next_node);
}

void MonteCarlo::UpdateNumSamples()
{
    ++_N;
}

std::shared_ptr<Command> MonteCarlo::GetBestMove()
{
    auto it = std::max_element(std::begin(_nodes), std::end(_nodes),
            [] (std::shared_ptr<MCMove> const lhs, std::shared_ptr<MCMove> const rhs) -> bool { 
                return lhs->GetN() < rhs->GetN(); });
    return (*it)->GetCommand();
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
