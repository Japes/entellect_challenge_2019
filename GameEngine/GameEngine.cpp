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

    ApplyPowerups();

    //at this point we'd ask the players for their next moves
}

void GameEngine::ApplyPowerups()
{
    for(auto& worm : _state->player1.worms) {
        auto powerupHere = _state->Cell_at(worm.position)->powerup;
        if(powerupHere != nullptr) {
            powerupHere->ApplyTo(&worm);
            _state->Cell_at(worm.position)->powerup = nullptr;
        }
    }

    //lame that this is duplicated
    for(auto& worm : _state->player2.worms) {
        auto powerupHere = _state->Cell_at(worm.position)->powerup;
        if(powerupHere != nullptr) {
            powerupHere->ApplyTo(&worm);
            _state->Cell_at(worm.position)->powerup = nullptr;
        }
    }
}

void GameEngine::Playthrough(bool player1, const Command& command)
{

}

std::vector<Command> GameEngine::GetValidMovesForWorm(bool player1)
{
    std::vector<Command> ret;
    return ret;
}
