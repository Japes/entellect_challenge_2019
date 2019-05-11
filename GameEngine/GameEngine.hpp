#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "GameState.hpp"
#include "Command.hpp"

class GameEngine
{
	public:
    GameEngine();
    GameEngine(std::shared_ptr<GameState> state);

    void AdvanceState(const Command& player1_command, const Command& player2_command);

    //GetValidMovesForWorm()

    private:
    std::shared_ptr<GameState> _state;

};

#endif
