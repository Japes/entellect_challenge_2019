#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "GameState.hpp"
#include "AllCommands.hpp"
#include <vector>
#include <random>
#include <functional>
#include "pcg_random.hpp"

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
    int Playthrough(bool player1, std::shared_ptr<Command> command, 
        std::function<std::shared_ptr<Command>(bool, std::shared_ptr<GameState>)> nextMoveFn, 
        std::function<float(bool, std::shared_ptr<GameState>)> evaluationFn,
        int radiusToConsider,
        int depth);
    GameResult GetResult();
    static GameResult GetResult(const std::shared_ptr<GameState> state);

    private:
    bool DoCommand(const Command& command, bool player1, bool valid);
    void ApplyPowerups();
    void UpdateWinCondition();

    std::shared_ptr<GameState> _state;
    GameResult _currentResult;
};

#endif
