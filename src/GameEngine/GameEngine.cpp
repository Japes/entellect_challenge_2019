#include "GameEngine.hpp"
#include "NextTurn.hpp"
#include "../Utilities/Utilities.hpp"
#include <algorithm>

GameEngine::GameEngine()
{
    NextTurn::Initialise();
}

GameEngine::GameEngine(GameStatePtr state) : 
    _state{state}
{
    NextTurn::Initialise();
}

GameEngine::GameEngine(std::shared_ptr<GameState> state) : 
    _state{state.get()}
{
    NextTurn::Initialise();
}

void GameEngine::AdvanceState(const Command& player1_command, const Command& player2_command)
{
    //std::cerr << "Advancing state with moves 1" << _state->player1.GetCurrentWorm()->id << ": " << player1_command.GetCommandString() << 
    //            " and 2" << _state->player2.GetCurrentWorm()->id << ": " << player2_command.GetCommandString() << std::endl;
    if(_currentResult.result != ResultType::IN_PROGRESS) {
        return; //nothing more to do here
    }

    ApplyLava();
    _state->ClearLavasRemovedThisRound();

    //thaw out worms
    _state->ForAllLiveWorms([&](Worm& worm) { worm.roundsUntilUnfrozen = std::max(worm.roundsUntilUnfrozen - 1, 0); });

    bool player1Good = player1_command.IsValid(true, _state);
    bool player2Good = player2_command.IsValid(false, _state);

    //determine which move should be executed first
    bool player1GoesFirst = player1_command.Order() <= player2_command.Order(); //they seem to favour player 1...
    bool movesValid = true;
    movesValid &= DoCommand(player1GoesFirst?player1_command:player2_command, player1GoesFirst, player1GoesFirst?player1Good:player2Good);
    movesValid &= DoCommand(!player1GoesFirst?player1_command:player2_command, !player1GoesFirst, !player1GoesFirst?player1Good:player2Good);

    Worm* p1worm = _state->player1.GetCurrentWorm();
    Worm* p2worm = _state->player2.GetCurrentWorm();
    if(!movesValid) {
        if(!player1Good) {
            std::cerr << "Invalid move by worm 1" << p1worm->id << ": " << player1_command.GetCommandString() << std::endl;
        }
        if(!player2Good) {
            std::cerr << "Invalid move by worm 2" << p2worm->id << ": " << player2_command.GetCommandString() << std::endl;
        }
    }

    _state->ClearDirtsDugThisRound();

    _state->ForAllLiveWorms([&](Worm& worm) { 
        if(worm.frozenThisRound) {
            worm.roundsUntilUnfrozen = GameConfig::technologistWorms.snowball.freezeDuration;
            worm.frozenThisRound = false;
        }
     });

    ProcessWormFlags(p1worm);
    ProcessWormFlags(p2worm);

    ApplyPowerups();
    GiveKillScores();

    _state->player1.UpdateCurrentWorm();
    _state->player2.UpdateCurrentWorm();

    ++_state->roundNumber;

    UpdateWinCondition();

    //at this point we'd ask the players for their next moves
}

void GameEngine::ProcessWormFlags(Worm* worm)
{
    worm->movedThisRound = false;
    worm->diedByLavaThisRound = false;
}

void GameEngine::GiveKillScores()
{
    _state->ForAllWorms([&](Worm& worm) {
        if(worm.IsDead()) {
            for(auto & attackingWorm : worm.lastAttackedBy ) {
                Player* attackingWormsPlayer = attackingWorm->playerId == 1? &_state->player1 : &_state->player2;
                if(attackingWorm->playerId == worm.playerId) {
                    attackingWormsPlayer->command_score -= GameConfig::scores.killShot;
                } else {
                    attackingWormsPlayer->command_score += GameConfig::scores.killShot;
                }
            }
        }
        worm.lastAttackedBy.clear();
    });    
}

void GameEngine::ApplyLava()
{
    _state->ForAllLiveWorms([&](Worm& worm) {
        if( worm.position.IsOnMap() && _state->LavaAt(worm.position)) {
            worm.health -= GameConfig::lavaDamage;
            if(worm.IsDead()) {
                worm.diedByLavaThisRound = true;
            }
        }
    });

    _state->player1.RecalculateHealth();
    _state->player2.RecalculateHealth();
}

//returns false if move is invalid
bool GameEngine::DoCommand(const Command& command, bool player1, bool valid)
{
    Player* player = _state->GetPlayer(player1);

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
    _currentResult = GetResult(_state);
}

void GameEngine::ApplyPowerups()
{
    _state->ForAllWorms([&](Worm& worm) {
        auto powerupHere = _state->PowerUp_at(worm.position);
        if(powerupHere != nullptr) {
            powerupHere->ApplyTo(&worm);
            _state->ClearPowerupAt(worm.position);
        }
    });

    _state->player1.RecalculateHealth();
    _state->player2.RecalculateHealth();
}

//do a random playthrough to the end and return:
//1 if player 1 wins
//0 if player 1 loses
//
// so returned scores are always in terms of player 1
//
//depth is how far to go before applying heuristic, -1 means play to end
//TODO pass in strategies for each player
//TODO pass in whether or not it should return binary or weights
float GameEngine::Playthrough(std::shared_ptr<Command> player1_Command, 
                            std::shared_ptr<Command> player2_Command, 
                            std::function<std::shared_ptr<Command>(bool, GameStatePtr)> nextMoveFn,
                            const EvaluatorBase* evaluator,
                            int depth,
                            int& numPlies)
{
    std::shared_ptr<Command> p1Command = player1_Command;
    std::shared_ptr<Command> p2Command = player2_Command;

    //run the playthrough
    auto evaluationBefore = evaluator->Evaluate(true, _state); //always in terms of player 1

    numPlies = 0;
    while(depth != 0 && _currentResult.result == ResultType::IN_PROGRESS) {
        AdvanceState(*p1Command.get(), *p2Command.get());
        p1Command = nextMoveFn(true, _state);
        p2Command = nextMoveFn(false, _state);
        --depth;
        ++numPlies;
    }

    auto evaluationAfter = evaluator->Evaluate(true, _state); //always in terms of player 1

    //evaluate the playthrough

    //first, best/worst possible outcome
    if(_currentResult.result != ResultType::IN_PROGRESS) {
        if(_currentResult.winningPlayer == &_state->player1) {
            return 1;
        } else {
            return 0;
        }
    }

    float bestPossible = evaluator->BestPossiblePerPly()*numPlies;
    float frac = (evaluationAfter - evaluationBefore) / bestPossible;
    // clamp scorediff to 0.25 - 0.75
    return Utilities::NormaliseTo(frac, 0.25, 0.75);
}

GameEngine::GameResult GameEngine::GetResult()
{
    return _currentResult;
}

GameEngine::GameResult GameEngine::GetResult(const GameStatePtr state)
{
    GameResult ret;

    bool anySurvivors1 = std::any_of(state->player1.worms.begin(), state->player1.worms.end(), [](Worm& w){return !w.IsDead();});
    bool anySurvivors2 = std::any_of(state->player2.worms.begin(), state->player2.worms.end(), [](Worm& w){return !w.IsDead();});

    //check for knockout
    if(anySurvivors1 != anySurvivors2) {
        ret.result = ResultType::FINISHED_KO;
        ret.winningPlayer = anySurvivors1 ? &state->player1 : &state->player2;
        ret.losingPlayer = (ret.winningPlayer == &state->player1) ? &state->player2 : &state->player1;
        //std::cerr << "knockout" << std::endl;
        return ret;
    }

    //check for disqualification
    if(state->player1.consecutiveDoNothingCount > GameConfig::maxDoNothings || state->player2.consecutiveDoNothingCount > GameConfig::maxDoNothings) {
        ret.result = ResultType::FINISHED_POINTS;
        ret.winningPlayer = state->player1.consecutiveDoNothingCount >= GameConfig::maxDoNothings? &state->player2 : &state->player1;
        ret.losingPlayer = (ret.winningPlayer == &state->player1) ? &state->player2 : &state->player1;
        //std::cerr << "disqualification" << std::endl;
        return ret;
    }

    //update currently winning player
    ret.winningPlayer = (state->player1.GetScore() > state->player2.GetScore()) ? &state->player1 : &state->player2;
    ret.losingPlayer = (ret.winningPlayer == &state->player1) ? &state->player2 : &state->player1;

    //check for tie
    if(state->roundNumber > GameConfig::maxRounds || (!anySurvivors1 && !anySurvivors2)) {
        //std::cerr << "rounds done" << std::endl;
        ret.result = ResultType::FINISHED_POINTS;
    }

    return ret;
}
