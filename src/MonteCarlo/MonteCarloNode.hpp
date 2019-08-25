#ifndef MCNODE_H
#define MCNODE_H

#include "PlayersMonteCarlo.hpp"
#include "Evaluators/EvaluatorBase.hpp"
#include <mutex>

class MonteCarloNode
{
    public:
    MonteCarloNode(std::shared_ptr<GameState> state, const EvaluatorBase* eval, int playthroughDepth, float c);
    int64_t AddPlaythrough();
    std::shared_ptr<Command> GetBestMove(bool player1);
    void PrintState(bool player1);

    private:
    std::shared_ptr<GameState> _state;
    PlayersMonteCarlo _player1_mc;
    PlayersMonteCarlo _player2_mc;
    std::mutex _mtx;
    const EvaluatorBase* _evaluator;
    int _playthroughDepth;

};


#endif
