#ifndef MCNODE_H
#define MCNODE_H

#include "PlayersMonteCarlo.hpp"
#include "Evaluators.hpp"
#include <mutex>
#include <unordered_map>

using childNodeID_t = std::pair<std::shared_ptr<Command>, std::shared_ptr<Command>>; //unique identifier for child nodes
using childNodeKey_t = std::string; //key in format required for map

class MonteCarloNode
{
    public:
    MonteCarloNode(std::shared_ptr<GameState> state, 
                    const std::vector<std::shared_ptr<Command>>& p1_cmds, const std::vector<std::shared_ptr<Command>>& p2_cmds,
                    EvaluationFn_t eval, 
                    int nodeDepth, int playthroughDepth, float c);

    MonteCarloNode(std::shared_ptr<GameState> state, EvaluationFn_t eval, int nodeDepth, int playthroughDepth, float c);

    void Promote();
    std::pair<float, float> AddPlaythrough(int& numplies, bool canMakeChild = true);

    std::shared_ptr<Command> GetBestMove(bool player1);
    std::shared_ptr<MonteCarloNode> GetChild(childNodeID_t id);
    std::shared_ptr<MonteCarloNode> TryGetComputedState(std::shared_ptr<GameState> state);
    bool StateEquals(std::shared_ptr<GameState> state);

    static childNodeKey_t GetChildKey(childNodeID_t id);

    //for debug
    int NumImmediateChildren();
    int TotalNumChildren();
    int MaxTreeDepth();
    int MinNumBranches();
    void PrintState(bool player1);

    private:
    std::shared_ptr<GameState> _state;
    PlayersMonteCarlo _player1_mc;
    PlayersMonteCarlo _player2_mc;
    std::mutex _mtx;
    EvaluationFn_t _evaluator;
    int _nodeDepth;
    int _playthroughDepth;
    float _c;
    std::pair<float, float> _terminalNodeEvaluation;

    std::unordered_map<childNodeKey_t, std::shared_ptr<MonteCarloNode>> _childNodes;

    std::shared_ptr<MonteCarloNode> GetOrCreateChild(childNodeID_t id, bool& createdNewOne);
    std::pair<float, float> DoAPlayout(std::shared_ptr<Command> p1Cmd, std::shared_ptr<Command> p2Cmd, int& numplies);
    std::pair<float, float> GetScore(std::shared_ptr<Command> p1Cmd, std::shared_ptr<Command> p2Cmd, int& numplies, bool canMakeChild);

};


#endif
