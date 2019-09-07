#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "GameState.hpp"
#include "AllCommands.hpp"
#include "Evaluators.hpp"
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
    GameEngine(GameStatePtr state);
    GameEngine(std::shared_ptr<GameState> state);

    void AdvanceState(const Command& player1_command, const Command& player2_command);
    std::pair<float, float> Playthrough(std::shared_ptr<Command> player1_Command, 
        std::shared_ptr<Command> player2_Command, 
        std::function<std::shared_ptr<Command>(bool, GameStatePtr)> nextMoveFn, 
        EvaluationFn_t evaluator,
        int depth,
        int& numPlies);
    GameResult GetResult();
    static GameResult GetResult(const GameStatePtr state);

    private:
    bool DoCommand(const Command& command, bool player1, bool valid);
    void ProcessWormFlags(Worm* worm);
    void ApplyPowerups();
    void GiveKillScores();
    void ApplyLava();
    void UpdateWinCondition();
    void SetupLava(unsigned roundNum);

    GameStatePtr _state;
    GameResult _currentResult;
};

#endif
