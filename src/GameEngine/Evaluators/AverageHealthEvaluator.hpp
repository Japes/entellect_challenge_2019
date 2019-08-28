#ifndef AVERAGE_HEALTH_EVALUATOR_H
#define AVERAGE_HEALTH_EVALUATOR_H

#include "EvaluatorBase.hpp"
#include "../GameEngine.hpp"

class AverageHealthEvaluator: public EvaluatorBase
{
	public:
    AverageHealthEvaluator()
    {
        _bestPossiblePerPly = GameConfig::agentWorms.banana.damage*2; //rough estimate...he hits his own guy, i hit his guy as well
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

        int numliveWormsMe = 0;
        state->ForAllLiveWorms(player1, [&](Worm& worm) {
            ++numliveWormsMe;
        });

        int numliveWormsHim = 0;
        state->ForAllLiveWorms(!player1, [&](Worm& worm) {
            ++numliveWormsHim;
        });

        return (myPlayer->health/numliveWormsMe) - (otherPlayer->health/numliveWormsHim);
    }
};

#endif
