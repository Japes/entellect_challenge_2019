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
    //std::cerr << "Advancing state with moves 1" << _state->player1.GetCurrentWorm()->id << ": " << player1_command.GetCommandString() << 
    //            " and 2" << _state->player2.GetCurrentWorm()->id << ": " << player2_command.GetCommandString() << std::endl;
    if(_currentResult.result != ResultType::IN_PROGRESS) {
        return; //nothing more to do here
    }

    SetupLava(_state->roundNumber);
    ApplyLava();

    //thaw out worms
    _state->ForAllLiveWorms([&](Worm& worm) { worm.roundsUntilUnfrozen = std::max(worm.roundsUntilUnfrozen - 1, 0); });

    bool player1Good = player1_command.IsValid(true, _state);
    bool player2Good = player2_command.IsValid(false, _state);


    //determine which move should be executed first
    bool player1GoesFirst = player1_command.Order() < player2_command.Order();
    bool movesValid = true;
    movesValid &= DoCommand(player1GoesFirst?player1_command:player2_command, player1GoesFirst, player1GoesFirst?player1Good:player2Good);
    movesValid &= DoCommand(!player1GoesFirst?player1_command:player2_command, !player1GoesFirst, !player1GoesFirst?player1Good:player2Good);

    if(!movesValid) {
        if(!player1Good) {
            std::cerr << "Invalid move by worm 1" << _state->player1.GetCurrentWorm()->id << ": " << player1_command.GetCommandString() << std::endl;
        }
        if(!player2Good) {
            std::cerr << "Invalid move by worm 2" << _state->player2.GetCurrentWorm()->id << ": " << player2_command.GetCommandString() << std::endl;
        }
    }

    _state->player1.GetCurrentWorm()->movedThisRound = false;
    _state->player1.GetCurrentWorm()->diedByLavaThisRound = false;
    _state->player2.GetCurrentWorm()->movedThisRound = false;
    _state->player2.GetCurrentWorm()->diedByLavaThisRound = false;

    ApplyPowerups();

    _state->player1.UpdateCurrentWorm();
    _state->player2.UpdateCurrentWorm();


    ++_state->roundNumber;

    UpdateWinCondition();

    //at this point we'd ask the players for their next moves
}

//TODO can make this much faster by precomputing lava for all rounds up front
void GameEngine::SetupLava(unsigned roundNum)
{
    int center = (GameConfig::mapSize - 1) / 2.0;
    Position mapCenter(center, center);

    float brStartRound = GameConfig::maxRounds * GameConfig::battleRoyaleStart;
    if (roundNum < brStartRound) {
        return;
    }

    float brEndRound = GameConfig::maxRounds * GameConfig::battleRoyaleEnd;
    float fullPercentageRange = (roundNum - brStartRound) / (brEndRound - brStartRound);
    //clamp tp [0,1]
    float currentProgress = fullPercentageRange > 0 ? fullPercentageRange : 0;
    currentProgress = currentProgress > 1 ? 1 : currentProgress;

    float safeAreaRadius = (GameConfig::mapSize / 2) * (1 - currentProgress);

    unsigned countLavas = 0;
    for(int x = 0; x < GameConfig::mapSize; ++x) {
        for(int y = 0; y < GameConfig::mapSize; ++y) {
            Position pos(x,y);
            if(mapCenter.EuclideanDistanceTo(pos) > (safeAreaRadius + 1)) {
                _state->AddLavaAt(pos);
                ++countLavas;
            }
        }
    }

    //std::cerr << "(" << __FUNCTION__ << ") roundNum: " << roundNum << " currentProgress: " << currentProgress 
    //            << " safeAreaRadius: " << safeAreaRadius << " countLavas: " << countLavas << std::endl;
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
//+1 if player wins
//-1 if player loses
//depth is how far to go before applying heuristic, -1 means play to end
//TODO pass in strategies for each player
//TODO pass in whether or not it should return binary or weights
float GameEngine::Playthrough(bool player1, std::shared_ptr<Command> command, 
                            std::function<std::shared_ptr<Command>(bool, std::shared_ptr<GameState>)> nextMoveFn,
                            std::function<float(bool, std::shared_ptr<GameState>)> evaluationFn,
                            int radiusToConsider,
                            int depth,
                            int& numPlies)
{
    //filter out worms too far away:
    Player* player = _state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();
    std::vector<Worm*> considered_worms = _state->WormsWithinDistance(worm->position, radiusToConsider);

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

    //first, best/worst possible outcome
    if(_currentResult.result != ResultType::IN_PROGRESS) {
        if(_currentResult.winningPlayer == player) {
            return 1;
        } else {
            return 0;
        }
    }

    //else clamp scorediff to 0.25 - 0.75
    //auto diff = evaluationAfter - evaluationBefore;
    //float bestPossible = static_cast<float>(numPlies) * 16; //should be safe
    //float frac = diff/bestPossible; //number between +- 1
    //std::cerr << "(" << __FUNCTION__ << ") frac: " << frac << std::endl;
    //float scaledFrac = frac/4; //number between +- 0.25
    //auto ret = scaledFrac + 0.5f; //number between 0.25 - 0.75
    //std::cerr << "(" << __FUNCTION__ << ") ret: " << ret << std::endl;
    //return ret;

    //return (((evaluationAfter - evaluationBefore) / (bestPossible)) / 4) + 0.5f; //this was for score

    float bestPossible = GameConfig::commandoWorms.initialHp + 
                            GameConfig::agentWorms.initialHp + 
                            GameConfig::technologistWorms.initialHp + 
                            GameConfig::healthPackHp*2;

    return (((evaluationAfter - evaluationBefore) / bestPossible ) / 4) + 0.5f; //this was for score
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
