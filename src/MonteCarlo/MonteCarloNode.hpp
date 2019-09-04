#ifndef MCNODE_H
#define MCNODE_H

#include "PlayersMonteCarlo.hpp"
#include "Evaluators/EvaluatorBase.hpp"
#include <mutex>
#include <unordered_map>

using childNodeID_t = std::pair<std::shared_ptr<Command>, std::shared_ptr<Command>>; //unique identifier for child nodes
using childNodeKey_t = std::string; //key in format required for map

class MonteCarloNode
{
    public:
    MonteCarloNode(std::shared_ptr<GameState> state, 
                    const std::vector<std::shared_ptr<Command>>& p1_cmds, const std::vector<std::shared_ptr<Command>>& p2_cmds,
                    const EvaluatorBase* eval, 
                    int nodeDepth, int playthroughDepth, float c);

    MonteCarloNode(std::shared_ptr<GameState> state, const EvaluatorBase* eval, int nodeDepth, int playthroughDepth, float c);

    void Promote();
    float AddPlaythrough(int& numplies, int& numplayouts);
    std::shared_ptr<Command> GetBestMove(bool player1);
    std::shared_ptr<MonteCarloNode> GetChild(childNodeID_t id);
    std::shared_ptr<MonteCarloNode> TryGetComputedState(std::shared_ptr<GameState> state);
    bool StateEquals(std::shared_ptr<GameState> state);

    static childNodeKey_t GetChildKey(childNodeID_t id);

    //for debug
    int NumChildren();
    int MaxTreeDepth();
    int MinNumBranches();
    void PrintState(bool player1);

    private:
    std::shared_ptr<GameState> _state;
    PlayersMonteCarlo _player1_mc;
    PlayersMonteCarlo _player2_mc;
    std::mutex _mtx;
    const EvaluatorBase* _evaluator;
    int _nodeDepth;
    int _playthroughDepth;
    float _c;

    std::unordered_map<childNodeKey_t, std::shared_ptr<MonteCarloNode>> _childNodes;

    std::shared_ptr<MonteCarloNode> GetOrCreateChild(childNodeID_t id);

};


#endif
