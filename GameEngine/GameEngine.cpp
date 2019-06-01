#include "GameEngine.hpp"
#include <algorithm>

std::vector<Position> GameEngine::_surroundingWormSpaces;

GameEngine::GameEngine()
{
}

GameEngine::GameEngine(std::shared_ptr<GameState> state) : 
    _state{state}
{
    // Seed with a real random value, if available
    pcg_extras::seed_seq_from<std::random_device> seed_source;
    // Make a random number engine 
    _rng = std::make_shared<pcg32>(seed_source);

    if(_player1Shoots.empty()) {
        _player1Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
        _player1Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S));
        _player1Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::E));
        _player1Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::W));
        _player1Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NW));
        _player1Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NE));
        _player1Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SW));
        _player1Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SE));
    }

    if(_player2Shoots.empty()) {
        _player2Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
        _player2Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S));
        _player2Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::E));
        _player2Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::W));
        _player2Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NW));
        _player2Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NE));
        _player2Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SW));
        _player2Shoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SE));
    }

    if(_surroundingWormSpaces.empty()) {
        _surroundingWormSpaces.push_back(Position(-1, -1));
        _surroundingWormSpaces.push_back(Position(0, -1));
        _surroundingWormSpaces.push_back(Position(1, -1));
        _surroundingWormSpaces.push_back(Position(1, 0));
        _surroundingWormSpaces.push_back(Position(1, 1));
        _surroundingWormSpaces.push_back(Position(0, 1));
        _surroundingWormSpaces.push_back(Position(-1, 1));
        _surroundingWormSpaces.push_back(Position(-1, 0));
    }
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
int GameEngine::Playthrough(bool player1, std::shared_ptr<Command> command, int depth)
{
    std::shared_ptr<Command> p1Command = player1? command : GetRandomValidMoveForWorm(true);
    std::shared_ptr<Command> p2Command = !player1? command : GetRandomValidMoveForWorm(false);

    while(depth != 0 && _currentResult.result == ResultType::IN_PROGRESS) {
        //std::cerr << "Advancing state with moves P1: " << p1Command->GetCommandString() << " and P2: " << p2Command->GetCommandString() << std::endl;
        AdvanceState(*p1Command.get(), *p2Command.get());
        p1Command = GetRandomValidMoveForWorm(true);
        p2Command = GetRandomValidMoveForWorm(false);
        --depth;
    }

    bool player1won = (_currentResult.winningPlayer == &_state->player1);
    if(player1 == player1won) {
        return 1;
    }
    return -1;
}

GameEngine::GameResult GameEngine::GetResult()
{
    return _currentResult;
}

//2 things are implied and left out from this return:
//1. "do nothing"
//2. "shoot" in all directions
std::vector<std::shared_ptr<Command>> GameEngine::GetValidMovesForWorm(bool player1, bool trimStupidMoves)
{
    std::vector<std::shared_ptr<Command>> ret;
    ret.reserve(8);

    Worm* worm = player1? _state->player1.GetCurrentWorm() : _state->player2.GetCurrentWorm();

    for(auto const & space : _surroundingWormSpaces) {
        Position wormSpace = worm->position + space;
        if(!wormSpace.IsOnMap() || _state->Cell_at(wormSpace)->worm != nullptr) {
            continue;
        }

        if(trimStupidMoves && wormSpace == worm->previous_position) {
            continue;
        }

        if(_state->Cell_at(wormSpace)->type == CellType::AIR) {
            ret.emplace_back(std::make_shared<TeleportCommand>(wormSpace));
        } else if(_state->Cell_at(wormSpace)->type == CellType::DIRT) {
            ret.emplace_back(std::make_shared<DigCommand>(wormSpace) );
        }
    }

    return ret;
}

std::vector<std::shared_ptr<Command>> GameEngine::GetSensibleShootsForWorm(bool player1)
{
    std::vector<std::shared_ptr<Command>> ret;
    ret.reserve(3);

    Player* player = player1 ? &_state->player1 : &_state->player2;
    Worm* worm = player->GetCurrentWorm();

    for(auto const & space : _surroundingWormSpaces) {
        auto shoot = std::make_shared<ShootCommand>(space);
        Worm* hitworm = shoot->WormOnTarget(player1, _state);
        if(hitworm != nullptr && std::none_of(player->worms.begin(), player->worms.end(), [&](Worm& w){return &w == hitworm;})) {
            ret.push_back(shoot);
        }
    }

    return ret;
}

std::shared_ptr<Command> GameEngine::GetRandomValidMoveForWorm(bool player1, bool trimStupidMoves)
{
    std::shared_ptr<Command> ret;

    //get random moves (teleport/dig) and shoots
    std::vector<std::shared_ptr<Command>> moves = GetValidMovesForWorm (player1, trimStupidMoves);
    std::vector<std::shared_ptr<Command>> shoots = trimStupidMoves? (GetSensibleShootsForWorm(player1)):(player1? _player1Shoots : _player2Shoots);

    //choose random one
    int totalNumMoves = moves.size() + shoots.size();
    std::uniform_int_distribution<int> uniform_dist(0, totalNumMoves-1);
    int mean = uniform_dist(*_rng.get());

    if(mean < static_cast<int>(moves.size())) {
        ret = moves[mean];
    } else {
        int index = mean - moves.size();
        if(player1) {
            ret = shoots[index];
        } else {
            ret = shoots[index];
        }
    }

    //std::cerr << "GameEngine::GetRandomValidMoveForWorm returning " << ret->GetCommandString() << std::endl;

    return ret;
}
