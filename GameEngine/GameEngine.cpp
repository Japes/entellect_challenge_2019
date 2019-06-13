#include "GameEngine.hpp"
#include <algorithm>
#include "NextTurn.hpp"

GameEngine::GameEngine()
{
    NextTurn::Initialise();
}

GameEngine::GameEngine(std::shared_ptr<GameState> state) : 
    _state{state}
{
    NextTurn::Initialise();
}

void GameEngine::AdvanceState(const Command& player1_command, const Command& player2_command)
{
    if(_currentResult.result != ResultType::IN_PROGRESS) {
        return; //nothing more to do here
    }

    //validate both guys first
    bool player1Good = player1_command.IsValid(true, _state);
    bool player2Good = player2_command.IsValid(false, _state);

    //determine which move should be executed first
    bool player1GoesFirst = player1_command.Order() < player2_command.Order();
    bool movesValid = true;
    movesValid &= DoCommand(player1GoesFirst?player1_command:player2_command, player1GoesFirst, player1GoesFirst?player1Good:player2Good);
    movesValid &= DoCommand(!player1GoesFirst?player1_command:player2_command, !player1GoesFirst, !player1GoesFirst?player1Good:player2Good);

    if(!movesValid) {
        std::cerr << "Invalid move found.  Player1 move: " << player1_command.GetCommandString() << " Player2 move: " << player2_command.GetCommandString() << std::endl;
    }

    _state->player1.UpdateCurrentWorm();
    _state->player2.UpdateCurrentWorm();

    ApplyPowerups();

    ++_state->roundNumber;

    UpdateWinCondition();

    //at this point we'd ask the players for their next moves
}

//returns false if move is invalid
bool GameEngine::DoCommand(const Command& command, bool player1, bool valid)
{
    Player* player = player1 ? &_state->player1 : &_state->player2;

    if(!valid) {
        player->command_score += GameConfig::scores.invalidCommand;
        ++player->consecutiveDoNothingCount;
        return false;
    }

    command.Execute(player1, _state);

    return true;
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
//depth is how far to go before applying heuristic, -1 means play to end
//TODO pass in strategies for each player
//TODO pass in whether or not it should return binary or weights
int GameEngine::Playthrough(bool player1, std::shared_ptr<Command> command, 
                            std::function<std::shared_ptr<Command>(bool, std::shared_ptr<GameState>)> nextMoveFn,
                            std::function<float(bool, std::shared_ptr<GameState>)> evaluationFn,
                            int radiusToConsider,
                            int depth)
{
    Player* myPlayer = player1 ? &_state->player1 : &_state->player2;
    Player* otherPlayer = myPlayer == &_state->player1 ? &_state->player2 : &_state->player1;

    std::shared_ptr<Command> p1Command = player1? command : nextMoveFn(true, _state);
    std::shared_ptr<Command> p2Command = !player1? command : nextMoveFn(false, _state);

    int pointDiffBefore = evaluationFn(player1, _state);// myPlayer->GetScore() - otherPlayer->GetScore();

    while(depth != 0 && _currentResult.result == ResultType::IN_PROGRESS) {
        //std::cerr << "Advancing state with moves P1: " << p1Command->GetCommandString() << " and P2: " << p2Command->GetCommandString() << std::endl;
        AdvanceState(*p1Command.get(), *p2Command.get());
        p1Command = nextMoveFn(true, _state);
        p2Command = nextMoveFn(false, _state);
        --depth;
    }

    int pointDiffAfter = evaluationFn(player1, _state);// myPlayer->GetScore() - otherPlayer->GetScore();

    bool won = (pointDiffAfter > pointDiffBefore);

    if(won) {
        return 1;
    }
    return -1;
}

GameEngine::GameResult GameEngine::GetResult()
{
    return _currentResult;
}
