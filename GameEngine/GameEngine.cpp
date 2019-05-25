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
    bool valid = command->IsValid();
    if(!valid) {
        command->GetPlayer()->command_score += GameConfig::scores.invalidCommand;
    }

    if( valid && command->Order() != 0) {
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
        //std::cerr << "knockout" << std::endl;
        return;
    }

    //check for disqualification
    if(_state->player1.consecutiveDoNothingCount >= GameConfig::maxDoNothings || _state->player2.consecutiveDoNothingCount >= GameConfig::maxDoNothings) {
        _currentResult.result = ResultType::FINISHED_KO;
        _currentResult.winningPlayer = _state->player1.consecutiveDoNothingCount >= GameConfig::maxDoNothings? &_state->player2 : &_state->player1;
        _currentResult.losingPlayer = (_currentResult.winningPlayer == &_state->player1) ? &_state->player2 : &_state->player1;
        //std::cerr << "disqualification" << std::endl;
        return;
    }

    //update currently winning player
    _currentResult.winningPlayer = (_state->player1.GetScore() > _state->player2.GetScore()) ? &_state->player1 : &_state->player2;
    _currentResult.losingPlayer = (_currentResult.winningPlayer == &_state->player1) ? &_state->player2 : &_state->player1;

    //check for tie
    if(_state->roundNumber > GameConfig::maxRounds || (!anySurvivors1 && !anySurvivors2)) {
        //std::cerr << "rounds done" << std::endl;
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

//do a random playthrough to the end and return:
//+1 if player wins
//-1 if player loses
void GameEngine::Playthrough(bool player1, const Command& command)
{

}

GameEngine::GameResult GameEngine::GetResult()
{
    return _currentResult;
}

//2 things are implied and left out from this return:
//1. "do nothing"
//2. "shoot" in any direction
std::vector<std::shared_ptr<Command>> GameEngine::GetValidMovesForWorm(bool player1)
{
    std::vector<std::shared_ptr<Command>> ret;
    Worm* worm = player1? _state->player1.GetCurrentWorm() : _state->player2.GetCurrentWorm();

    std::vector<Position> surroundingSpaces;
    surroundingSpaces.push_back(worm->position + Position(-1, -1));
    surroundingSpaces.push_back(worm->position + Position(0, -1));
    surroundingSpaces.push_back(worm->position + Position(1, -1));
    surroundingSpaces.push_back(worm->position + Position(1, 0));
    surroundingSpaces.push_back(worm->position + Position(1, 1));
    surroundingSpaces.push_back(worm->position + Position(0, 1));
    surroundingSpaces.push_back(worm->position + Position(-1, 1));
    surroundingSpaces.push_back(worm->position + Position(-1, 0));

    for(auto const & space : surroundingSpaces) {
        if(!space.IsOnMap() ||
         _state->Cell_at(space)->worm != nullptr) {
            continue;
        }

        if(_state->Cell_at(space)->type == CellType::AIR) {
            ret.push_back(std::make_shared<TeleportCommand>(player1, _state, space) );
        } else if(_state->Cell_at(space)->type == CellType::DIRT) {
            ret.push_back(std::make_shared<DigCommand>(player1, _state, space) );
        }

    }

    return ret;
}
