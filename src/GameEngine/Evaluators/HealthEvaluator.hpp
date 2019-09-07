#ifndef HEALTH_EVALUATOR_H
#define HEALTH_EVALUATOR_H

#include "EvaluatorBase.hpp"
#include "../GameEngine.hpp"

class HealthEvaluator: public EvaluatorBase
{
	public:
    HealthEvaluator()
    {
        _bestPossible = 470; //rough estimate...
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

        return myPlayer->health - otherPlayer->health;
    }
};

#endif