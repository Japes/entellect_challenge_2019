#ifndef MCNODE_H
#define MCNODE_H

#include "PlayersMonteCarlo.hpp"
#include "Evaluators/EvaluatorBase.hpp"
#include <mutex>
#include <unordered_map>

using childNodeKey_t = std::string;

class MonteCarloNode
{
    public:
    MonteCarloNode(std::shared_ptr<GameState> state, const EvaluatorBase* eval, int nodeDepth, int playthroughDepth, float c);
    float AddPlaythrough(int& numplies);
    std::shared_ptr<Command> GetBestMove(bool player1);

    static childNodeKey_t GetChildKey(std::shared_ptr<MCMove> p1Move, std::shared_ptr<MCMove> p2Move);

    //for debug
    int NumChildren();
    int NumBranches();
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

    std::shared_ptr<MonteCarloNode> GetOrCreateChild(std::shared_ptr<MCMove> p1Move, std::shared_ptr<MCMove> p2Move);

};


#endif
