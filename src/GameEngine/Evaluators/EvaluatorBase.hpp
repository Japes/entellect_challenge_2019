#ifndef EVALUATOR_BASE_H
#define EVALUATOR_BASE_H

#include "GameState.hpp"
#include <memory>

class EvaluatorBase
{
	public:
    virtual float Evaluate (bool player1, GameStatePtr state) const = 0;

     //maximum that result of Evaluate() could change each time a game state is moved forward 1 ply
    float BestPossible () const 
    {
        return _bestPossible;
    };

    protected:
    float _bestPossible;
};

#endif
