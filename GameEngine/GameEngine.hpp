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
    int Playthrough(bool player1, std::shared_ptr<Command> command, std::function<std::shared_ptr<Command>(bool, std::shared_ptr<GameState>)> nextMoveFn, bool hardWin, int depth);
    static std::vector<std::shared_ptr<Command>> GetValidTeleportDigsForWorm(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves = false);
    static std::shared_ptr<Command> GetRandomValidMoveForWorm(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves = false);
    static std::vector<std::shared_ptr<Command>> GetSensibleShootsForWorm(bool player1, std::shared_ptr<GameState> state);
    GameResult GetResult();

    static std::vector<std::shared_ptr<Command>> _playerShoots;
    
    private:
    bool DoCommand(const Command& command, bool player1, bool valid);
    void ApplyPowerups();
    void UpdateWinCondition();

    std::shared_ptr<GameState> _state;
    GameResult _currentResult;

    static std::shared_ptr<pcg32> _rng;

    static std::vector<Position> _surroundingWormSpaces;

};

#endif
