#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "GameState.hpp"
#include "Command.hpp"
#include <vector>

class GameEngine
{
	public:

    enum class ResultType:int
    {
        IN_PROGRESS,
        FINISHED_KO,
        FINISHED_POINTS
    };

    struct GameResult
    {
        Player* winningPlayer = nullptr; //these can be null if game is in progress
        Player* losingPlayer = nullptr;
        ResultType result = ResultType::IN_PROGRESS;
    };

    GameEngine();
    GameEngine(std::shared_ptr<GameState> state);

    void AdvanceState(const Command& player1_command, const Command& player2_command);
    void Playthrough(bool player1, const Command& command);
    std::vector<Command> GetValidMovesForWorm(bool player1);
    GameResult GetResult();

    private:
    void ApplyPowerups();
    void UpdateWinCondition();

    std::shared_ptr<GameState> _state;
    GameResult _currentResult;

};

#endif
