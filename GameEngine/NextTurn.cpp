#include "NextTurn.hpp"
#include <algorithm>

std::shared_ptr<pcg32> NextTurn::_rng;
std::vector<std::shared_ptr<Command>> NextTurn::_playerShoots;
std::vector<Position> NextTurn::_surroundingWormSpaces;
    

void NextTurn::Initialise()
{
    if(_playerShoots.empty()) {
        // Seed with a real random value, if available
        pcg_extras::seed_seq_from<std::random_device> seed_source;
        // Make a random number engine 
        _rng = std::make_shared<pcg32>(seed_source);

        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::E));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::W));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NW));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NE));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SW));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SE));
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

//2 things are implied and left out from this return:
//1. "do nothing"
//2. "shoot" in all directions
std::vector<std::shared_ptr<Command>> NextTurn::GetValidTeleportDigsForWorm(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    std::vector<std::shared_ptr<Command>> ret;
    ret.reserve(8);

    Worm* worm = player1? state->player1.GetCurrentWorm() : state->player2.GetCurrentWorm();

    for(auto const & space : _surroundingWormSpaces) {
        Position wormSpace = worm->position + space;
        if(!wormSpace.IsOnMap() || state->Cell_at(wormSpace)->worm != nullptr) {
            continue;
        }

        if(state->Cell_at(wormSpace)->type == CellType::AIR) {
            ret.emplace_back(std::make_shared<TeleportCommand>(wormSpace));
        } else if(state->Cell_at(wormSpace)->type == CellType::DIRT) {
            ret.emplace_back(std::make_shared<DigCommand>(wormSpace) );
        }
    }

    return ret;
}

std::vector<std::shared_ptr<Command>> NextTurn::GetSensibleShootsForWorm(bool player1, std::shared_ptr<GameState> state)
{
    std::vector<std::shared_ptr<Command>> ret;
    ret.reserve(3);

    Player* player = player1 ? &state->player1 : &state->player2;

    for(auto const & space : _surroundingWormSpaces) {
        Worm* hitworm = ShootCommand::WormOnTarget(player1, state, space);
        if(hitworm != nullptr && std::none_of(player->worms.begin(), player->worms.end(), [&](Worm& w){return &w == hitworm;})) {
            auto shoot = std::make_shared<ShootCommand>(space);
            ret.push_back(shoot);
        }
    }

    return ret;
}

std::shared_ptr<Command> NextTurn::GetRandomValidMoveForWorm(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    std::shared_ptr<Command> ret;

    //get random moves (teleport/dig) and shoots
    std::vector<std::shared_ptr<Command>> moves = GetValidTeleportDigsForWorm (player1, state, trimStupidMoves);
    std::vector<std::shared_ptr<Command>> shoots = trimStupidMoves? (GetSensibleShootsForWorm(player1, state)):_playerShoots;

    //choose random one
    int totalNumMoves = moves.size() + shoots.size();

    if(totalNumMoves == 0) {
        return std::make_shared<DoNothingCommand>();
    }

    std::uniform_int_distribution<int> uniform_dist(0, totalNumMoves-1);
    int mean = uniform_dist(*_rng.get());

    if(mean < static_cast<int>(moves.size())) {
        ret = moves[mean];
    } else {
        int index = mean - moves.size();
        ret = shoots[index];
    }

    //std::cerr << "GameEngine::GetRandomValidMoveForWorm returning " << ret->GetCommandString() << std::endl;

    return ret;
}
