#ifndef EVALUATION_FUNCTIONS_H
#define EVALUATION_FUNCTIONS_H

#include "GameState.hpp"
#include "Command.hpp"
#include <memory>

class EvaluationFunctions
{
	public:
    static float ScoreComparison (bool player1, GameStatePtr state);
    static float HealthComparison (bool player1, GameStatePtr state);
};

#endif
