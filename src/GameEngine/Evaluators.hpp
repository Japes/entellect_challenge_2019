#ifndef EVALUATORS_H
#define EVALUATORS_H

#include "GameState.hpp"
#include <memory>

using EvaluationFn_t = std::function<float(bool, GameStatePtr)>;

//all evaluation functions must return a value between 0 and 1, with 0 the worst for player1 and 1 the best

class Evaluators
{
	public:
    
    static float AveHpScore (bool player1, GameStatePtr state);
    static float AverageHealth (bool player1, GameStatePtr state);
    static float Health (bool player1, GameStatePtr state);
    static float MaxHpScore (bool player1, GameStatePtr state);
    static float RushHealth (bool player1, GameStatePtr state);
    static float Score (bool player1, GameStatePtr state);

    static float GetBananaBonus(int numBananas, int roundNumber); //not an evaluator

};

#endif
