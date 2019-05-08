#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "GameState.hpp"
#include "Command.hpp"

class GameEngine
{
	public:
    GameEngine();
    GameEngine(GameState state);

    void AdvanceState(const Command& player1_command, const Command& player2_command);

    private:
    GameState _state;

};

#endif
