#ifndef MAXHPSCORE_EVALUATOR_H
#define MAXHPSCORE_EVALUATOR_H

#include "EvaluatorBase.hpp"
#include "../GameEngine.hpp"

class MaxHpScoreEvaluator: public EvaluatorBase
{
	public:
    MaxHpScoreEvaluator()
    {
        _bestPossible = 170 + 1000/10; //rough estimate
    }

    float Evaluate (bool player1, GameStatePtr state) const override
    {
        Player* myPlayer = state->GetPlayer(player1);
        Player* otherPlayer = state->GetPlayer(!player1);

        int maxHealthMe = 0;
        state->ForAllLiveWorms(player1, [&](Worm& worm) {
            if(worm.health > maxHealthMe) {
                maxHealthMe = worm.health;
            }
        });

        int maxHealthHim = 0;
        state->ForAllLiveWorms(!player1, [&](Worm& worm) {
            if(worm.health > maxHealthHim) {
                maxHealthHim = worm.health;
            }
        });

        float healthdiff = maxHealthMe - maxHealthHim;
        float scorediff =  myPlayer->GetScore() - otherPlayer->GetScore();

        return healthdiff + scorediff/10;
    }
};

#endif
