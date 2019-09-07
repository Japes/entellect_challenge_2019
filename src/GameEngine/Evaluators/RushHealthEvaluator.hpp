#ifndef RUSH_HEALTH_EVALUATOR_H
#define RUSH_HEALTH_EVALUATOR_H

#include "EvaluatorBase.hpp"
#include "../GameEngine.hpp"

class RushHealthEvaluator: public EvaluatorBase
{
	public:
    RushHealthEvaluator()
    {
        _bestPossible = 17.0f;
    }

    float Evaluate (bool player1, GameStatePtr state) const override
    {
        Player* myPlayer = state->GetPlayer(player1);
        Worm* worm = myPlayer->GetWormById(1);

        if(worm == nullptr) {
            return 0;
        }
        
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

        return (worstDist - minDist);
    }
};

#endif
