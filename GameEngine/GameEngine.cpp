#include "GameEngine.hpp"

GameEngine::GameEngine()
{

}

GameEngine::GameEngine(std::shared_ptr<GameState> state) : _state{state}
{

}

void GameEngine::AdvanceState(const Command& player1_command, const Command& player2_command)
{
    const Command* firstCommand = &player1_command;
    const Command* secondCommand = &player2_command;

    //determine which move should be executed first
    if(player2_command.Order() < player1_command.Order()) {
        firstCommand = &player2_command;
        secondCommand = &player1_command;
    }

    if(firstCommand->IsValid()) {
        firstCommand->Execute();
    } else {
        ++firstCommand->GetPlayer()->consecutiveDoNothingCount;
    }

    if(secondCommand->IsValid()) {
        secondCommand->Execute();
    } else {
        ++secondCommand->GetPlayer()->consecutiveDoNothingCount;
    }

    _state->player1.UpdateCurrentWorm();
    _state->player2.UpdateCurrentWorm();

    //at this point we'd ask the players for their next moves
}
