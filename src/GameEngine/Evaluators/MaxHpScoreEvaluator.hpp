#ifndef MAXHPSCORE_EVALUATOR_H
#define MAXHPSCORE_EVALUATOR_H

#include "EvaluatorBase.hpp"
#include "../GameEngine.hpp"

class MaxHpScoreEvaluator: public EvaluatorBase
{
	public:
    MaxHpScoreEvaluator()
    {
        _bestPossible = (170*3) + (900/10); //rough estimate
    }

    //want this to be low if round is early and bombs are few
    //max if roundnumber > 350
    //max worth around 80 health
    float GetBananaBonus(int numBananas, int roundNumber) const
    {
        float max = 80.0f;
        if(roundNumber > 350) {
            return 0;
        }

        float frac = ( numBananas * (350.0f - roundNumber) ) / (3.0f*350.0f);
        return max*frac;
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
        int numWormsHim = 0;
        state->ForAllLiveWorms(!player1, [&](Worm& worm) {
            ++numWormsHim;
            if(worm.health > maxHealthHim) {
                maxHealthHim = worm.health;
            }
        });

        float healthdiff = maxHealthMe - maxHealthHim;
        float scorediff =  myPlayer->GetScore() - otherPlayer->GetScore();

        //want to encourage holding on to the banana
        float bananaBonus = GetBananaBonus(myPlayer->worms[1].banana_bomb_count, state->roundNumber);

        return (healthdiff*numWormsHim) + (scorediff/10) + bananaBonus;
    }
};

#endif
