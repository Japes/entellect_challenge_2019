#include "EvaluationFunctions.hpp"

float EvaluationFunctions::ScoreComparison (bool player1, std::shared_ptr<GameState> state)
{
    Player* myPlayer = player1 ? &state->player1 : &state->player2;
    Player* otherPlayer = myPlayer == &state->player1 ? &state->player2 : &state->player1;

    return myPlayer->GetScore() - otherPlayer->GetScore();
}