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
    //std::cerr << "Advancing state with moves P1: " << player1_command.GetCommandString() << " and P2: " << player2_command.GetCommandString() << std::endl;
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
#ifdef EXCEPTION_ON_ERROR
        throw std::runtime_error("Invalid move found!");
#endif
    }

    _state->player1.GetCurrentWorm()->movedThisRound = false;
    _state->player2.GetCurrentWorm()->movedThisRound = false;

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
    auto worms = _state->AllWorms();
    for(auto& worm : worms) {
        auto powerupHere = _state->Cell_at(worm->position)->powerup;
        if(powerupHere != nullptr) {
            powerupHere->ApplyTo(worm);
            _state->Cell_at(worm->position)->powerup = nullptr;
            _state->player1.RecalculateHealth();
            _state->player2.RecalculateHealth();
        }
    }
}

//return all worms if dist is -1
std::vector<Worm*> GameEngine::WormsWithinDistance(Position pos, int dist)
{
    std::vector<Worm*> ret;

    auto worms = _state->AllWorms();
    for(auto& worm : worms) {
        if(dist < 0 || pos.EuclideanDistanceTo(worm->position) <= dist) {
            ret.push_back(worm);
        }
    }
    return ret;
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
                            int depth,
                            int& numPlies)
{
    //filter out worms too far away:
    Player* player = _state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();
    std::vector<Worm*> considered_worms = WormsWithinDistance(worm->position, radiusToConsider);

    auto filteredNextMoveFn = [&] (bool player1, std::shared_ptr<GameState> state) -> std::shared_ptr<Command> {
        Player* player = state->GetPlayer(player1);
        Worm* worm = player->GetCurrentWorm();
        if(std::end(considered_worms) == std::find(std::begin(considered_worms), std::end(considered_worms), worm)) {
            return std::make_shared<DoNothingCommand>();
        }
        return nextMoveFn(player1, state);
    };

    std::shared_ptr<Command> p1Command = player1? command : filteredNextMoveFn(true, _state);
    std::shared_ptr<Command> p2Command = !player1? command : filteredNextMoveFn(false, _state);

    //run the playthrough
    auto evaluationBefore = evaluationFn(player1, _state);

    numPlies = 0;
    while(depth != 0 && _currentResult.result == ResultType::IN_PROGRESS) {
        
        if(radiusToConsider >= 0) {
            //don't allow disqualification if we're filtering out worms
            _state->player1.consecutiveDoNothingCount = 0;
            _state->player2.consecutiveDoNothingCount = 0;
        }

        AdvanceState(*p1Command.get(), *p2Command.get());
        p1Command = filteredNextMoveFn(true, _state);
        p2Command = filteredNextMoveFn(false, _state);
        --depth;
        ++numPlies;
    }

    auto evaluationAfter = evaluationFn(player1, _state);

    //evaluate the playthrough
    bool won = (evaluationAfter > evaluationBefore);

    if(won) {
        return 1;
    }
    return -1;
}

GameEngine::GameResult GameEngine::GetResult()
{
    return _currentResult;
}

GameEngine::GameResult GameEngine::GetResult(const std::shared_ptr<GameState> state)
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
