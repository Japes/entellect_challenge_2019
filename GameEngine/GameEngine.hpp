#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "GameState.hpp"
#include "AllCommands.hpp"
#include <vector>
#include <random>
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
    int Playthrough(bool player1, std::shared_ptr<Command> command, int depth = -1);
    std::vector<std::shared_ptr<Command>> GetValidMovesForWorm(bool player1);
    std::shared_ptr<Command> GetRandomValidMoveForWorm(bool player1);
    GameResult GetResult();

    private:
    void DoCommand(const Command* command);
    void ApplyPowerups();
    void UpdateWinCondition();

    std::shared_ptr<GameState> _state;
    GameResult _currentResult;

    std::shared_ptr<pcg32> _rng;

    std::vector<std::shared_ptr<Command>> _player1Shoots;
    std::vector<std::shared_ptr<Command>> _player2Shoots;

    static std::vector<Position> _surroundingWormSpaces;

};

#endif
