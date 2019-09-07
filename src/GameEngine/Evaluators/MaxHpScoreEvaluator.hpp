#ifndef MAXHPSCORE_EVALUATOR_H
#define MAXHPSCORE_EVALUATOR_H

#include "EvaluatorBase.hpp"
#include "../GameEngine.hpp"

class MaxHpScoreEvaluator: public EvaluatorBase
{
	public:
    MaxHpScoreEvaluator()
    {
        _bestPossible = 170 + 900/10; //rough estimate
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
        Worm* worm = myPlayer->GetCurrentWorm();

        float magic = 1000000;
        float minDist = magic;
        for(auto const& pos : state->GetHealthPackPos()) {
            int dist = worm->position.MovementDistanceTo(pos);
            if(dist < minDist) {
                minDist = dist;
            }
        }

        if(minDist == magic) {
            return 0;
        }

        float worstDist = 17.0f;
        return (worstDist - minDist)/17.0f;
    }
};

#endif
