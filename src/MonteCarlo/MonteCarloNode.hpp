#ifndef MCNODE_H
#define MCNODE_H

#include "PlayersMonteCarlo.hpp"
#include "Evaluators/EvaluatorBase.hpp"
#include <mutex>
#include <map>

using childNodeKey_t = std::pair<MCMove*, MCMove*>;

class MonteCarloNode
{
    public:
    MonteCarloNode(std::shared_ptr<GameState> state, const EvaluatorBase* eval, int nodeDepth, int playthroughDepth, float c);
    float AddPlaythrough(int& numplies);
    std::shared_ptr<Command> GetBestMove(bool player1);

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

    std::map<childNodeKey_t, std::shared_ptr<MonteCarloNode>> _childNodes;

};


#endif
