#include "GameEngine.hpp"
#include <algorithm>

GameEngine::GameEngine()
{
}

GameEngine::GameEngine(std::shared_ptr<GameState> state) : 
    _state{state}
{

}

void GameEngine::AdvanceState(const Command& player1_command, const Command& player2_command)
{
    if(_currentResult.result != ResultType::IN_PROGRESS) {
        return; //nothing more to do here
    }

    const Command* firstCommand = &player1_command;
    const Command* secondCommand = &player2_command;

    //determine which move should be executed first
    if(player2_command.Order() < player1_command.Order()) {
        firstCommand = &player2_command;
        secondCommand = &player1_command;
    }

    DoCommand(firstCommand);
    DoCommand(secondCommand);

    _state->player1.UpdateCurrentWorm();
    _state->player2.UpdateCurrentWorm();

    ApplyPowerups();

    ++_state->roundNumber;

    UpdateWinCondition();

    //at this point we'd ask the players for their next moves
}

void GameEngine::DoCommand(const Command* command)
{
    if(command->IsValid() && command->Order() != 0) {
        command->Execute();
        command->GetPlayer()->consecutiveDoNothingCount = 0;
    } else {
        ++command->GetPlayer()->consecutiveDoNothingCount;
    }
}

void GameEngine::UpdateWinCondition()
{
    bool anySurvivors1 = std::any_of(_state->player1.worms.begin(), _state->player1.worms.end(), [](Worm& w){return !w.IsDead();});
    bool anySurvivors2 = std::any_of(_state->player2.worms.begin(), _state->player2.worms.end(), [](Worm& w){return !w.IsDead();});

    //check for knockout
    if(anySurvivors1 != anySurvivors2) {
        _currentResult.result = ResultType::FINISHED_KO;
        _currentResult.winningPlayer = anySurvivors1 ? &_state->player1 : &_state->player2;
        _currentResult.losingPlayer = (_currentResult.winningPlayer == &_state->player1) ? &_state->player2 : &_state->player1;
        //std::cout << "knockout" << std::endl;
        return;
    }

    //check for disqualification
    if(_state->player1.consecutiveDoNothingCount >= GameConfig::maxDoNothings || _state->player2.consecutiveDoNothingCount >= GameConfig::maxDoNothings) {
        _currentResult.result = ResultType::FINISHED_KO;
        _currentResult.winningPlayer = _state->player1.consecutiveDoNothingCount >= GameConfig::maxDoNothings? &_state->player2 : &_state->player1;
        _currentResult.losingPlayer = (_currentResult.winningPlayer == &_state->player1) ? &_state->player2 : &_state->player1;
        //std::cout << "disqualification" << std::endl;
        return;
    }

    //update currently winning player
    if(_state->player1.score == _state->player1.score) {
        _currentResult.winningPlayer = _currentResult.losingPlayer = nullptr;
    } else {
        _currentResult.winningPlayer = (_state->player1.score > _state->player2.score) ? &_state->player1 : &_state->player2;
        _currentResult.losingPlayer = (_currentResult.winningPlayer == &_state->player1) ? &_state->player2 : &_state->player1;
    }

    //check for tie
    if(_state->roundNumber > GameConfig::maxRounds || (!anySurvivors1 && !anySurvivors2)) {
        //std::cout << "rounds done" << std::endl;
        _currentResult.result = ResultType::FINISHED_POINTS;
    }
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

GameEngine::GameResult GameEngine::GetResult()
{
    return _currentResult;
}

std::vector<Command> GameEngine::GetValidMovesForWorm(bool player1)
{
    std::vector<Command> ret;
    return ret;
}
