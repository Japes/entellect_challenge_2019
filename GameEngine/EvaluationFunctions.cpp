#include "EvaluationFunctions.hpp"
#include "GameEngine.hpp"
#include <limits>

float EvaluationFunctions::ScoreComparison (bool player1, std::shared_ptr<GameState> state)
{
    Player* myPlayer = player1 ? &state->player1 : &state->player2;
    Player* otherPlayer = myPlayer == &state->player1 ? &state->player2 : &state->player1;

    GameEngine::GameResult currentRes = GameEngine::GetResult(state);
    if(currentRes.result != GameEngine::ResultType::IN_PROGRESS) {
        if(currentRes.winningPlayer == myPlayer) {
            return std::numeric_limits<float>::max();
        } else if(currentRes.winningPlayer == otherPlayer) {
            return std::numeric_limits<float>::min();
        }
    }

    return myPlayer->GetScore() - otherPlayer->GetScore();
}