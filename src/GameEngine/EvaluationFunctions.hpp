#ifndef EVALUATION_FUNCTIONS_H
#define EVALUATION_FUNCTIONS_H

#include "GameState.hpp"
#include <memory>

class EvaluationFunctions
{
	public:
    static float ScoreComparison (bool player1, std::shared_ptr<GameState> state);
    static float HealthComparison (bool player1, std::shared_ptr<GameState> state);
};

#endif
