#include "GameEngine.hpp"

GameEngine::GameEngine()
{

}

GameEngine::GameEngine(GameState state) : _state{state}
{

}

void GameEngine::AdvanceState(const Command& player1_command, const Command& player2_command)
{
    //determine which move should be executed first
    if(player1_command.Order() <= player2_command.Order()) {
        player1_command.Execute(true, _state);
        player2_command.Execute(false, _state);
    } else {
        player2_command.Execute(false, _state);
        player1_command.Execute(true, _state);
    }

    _state.player1.UpdateCurrentWorm();
    _state.player2.UpdateCurrentWorm();

    //at this point we'd ask the players for their next moves
}
