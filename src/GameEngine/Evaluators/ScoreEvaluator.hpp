#ifndef SCORE_EVALUATOR_H
#define SCORE_EVALUATOR_H

#include "EvaluatorBase.hpp"
#include "../GameEngine.hpp"

class ScoreEvaluator: public EvaluatorBase
{
	public:

    ScoreEvaluator()
    {
        _bestPossiblePerPly = GameConfig::agentWorms.banana.damage*2*2; //rough estimate...he hits his own guy, i hit his guy as well
    }

    float Evaluate (bool player1, GameStatePtr state) const override
    {
        Player* myPlayer = state->GetPlayer(player1);
        Player* otherPlayer = state->GetPlayer(!player1);

        GameEngine::GameResult currentRes = GameEngine::GetResult(state);
        if(currentRes.result != GameEngine::ResultType::IN_PROGRESS) {
            if(currentRes.winningPlayer == myPlayer) {
                return std::numeric_limits<float>::max();
            } else if(currentRes.winningPlayer == otherPlayer) {
                return -std::numeric_limits<float>::max();
            }
        }

        return myPlayer->GetScore() - otherPlayer->GetScore();
    }
};

#endif
