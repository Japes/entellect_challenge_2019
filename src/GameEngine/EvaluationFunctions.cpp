#include "EvaluationFunctions.hpp"
#include "GameEngine.hpp"
#include <limits>

float EvaluationFunctions::ScoreComparison (bool player1, std::shared_ptr<GameState> state)
{
    Player* myPlayer = state->GetPlayer(player1);
    Player* otherPlayer = state->GetPlayer(!player1);

    GameEngine::GameResult currentRes = GameEngine::GetResult(state);
    if(currentRes.result != GameEngine::ResultType::IN_PROGRESS) {
        if(currentRes.winningPlayer == myPlayer) {
            return std::numeric_limits<float>::max();
        } else if(currentRes.winningPlayer == otherPlayer) {
            return -std::numeric_limits<float>::max();
        }
    }

    return myPlayer->GetScore() - otherPlayer->GetScore();
}

float EvaluationFunctions::HealthComparison (bool player1, std::shared_ptr<GameState> state)
{
    Player* myPlayer = state->GetPlayer(player1);
    Player* otherPlayer = state->GetPlayer(!player1);

    GameEngine::GameResult currentRes = GameEngine::GetResult(state);
    if(currentRes.result != GameEngine::ResultType::IN_PROGRESS) {
        if(currentRes.winningPlayer == myPlayer) {
            return std::numeric_limits<float>::max();
        } else if(currentRes.winningPlayer == otherPlayer) {
            return -std::numeric_limits<float>::max();
        }
    }

    return myPlayer->health - otherPlayer->health;
}