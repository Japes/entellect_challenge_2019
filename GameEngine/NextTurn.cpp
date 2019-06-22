#include "NextTurn.hpp"
#include <algorithm>
#include <bitset>

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
        //the order of these guys is important (see GetValidTeleportDigs)
        _surroundingWormSpaces.push_back(Position(-1, -1));
        _surroundingWormSpaces.push_back(Position(0, -1));
        _surroundingWormSpaces.push_back(Position(1, -1));
        _surroundingWormSpaces.push_back(Position(-1, 0));
        _surroundingWormSpaces.push_back(Position(1, 0));
        _surroundingWormSpaces.push_back(Position(-1, 1));
        _surroundingWormSpaces.push_back(Position(0, 1));
        _surroundingWormSpaces.push_back(Position(1, 1));
    }
}

//returns a bitfield representing the valid directions to move/dig
//bits are set as follows:
// 0 1 2
// 3 w 4
// 5 6 7
uint8_t NextTurn::GetValidTeleportDigs(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    uint8_t ret = 0;
    Worm* worm = player1? state->player1.GetCurrentWorm() : state->player2.GetCurrentWorm();

    for(auto const & space : _surroundingWormSpaces) {
        ret <<= 1;
        Position wormSpace{worm->position + space};
        if(wormSpace.IsOnMap() && state->Cell_at(wormSpace)->worm == nullptr && state->Cell_at(wormSpace)->type != CellType::DEEP_SPACE) {
            ret += 1;
        }
    }

    return ret;
}

//returns a bitfield representing the valid directions to shoot
//bits are set as follows:
// 0 1 2
// 3 w 4
// 5 6 7
std::vector<std::shared_ptr<Command>> NextTurn::GetShootsForWorm(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    if (!trimStupidMoves) {
        return _playerShoots;
    }

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

std::shared_ptr<Command> NextTurn::GetTeleportDig(bool player1, std::shared_ptr<GameState> state, unsigned index)
{
    Worm* worm = player1? state->player1.GetCurrentWorm() : state->player2.GetCurrentWorm();
    Position targetPos{worm->position + _surroundingWormSpaces[index]};

    if(state->Cell_at(targetPos)->worm == nullptr) {
        if(state->Cell_at(targetPos)->type == CellType::AIR) {
            return std::make_shared<TeleportCommand>(targetPos);
        } else if(state->Cell_at(targetPos)->type == CellType::DIRT) {
            return std::make_shared<DigCommand>(targetPos);
        }
    }

    std::cerr << "(" << __FUNCTION__ << ") index specified that isn't a valid teleport or dig.  Doing nothing." << std::endl;
    return std::make_shared<DoNothingCommand>();
}

std::shared_ptr<Command> NextTurn::GetRandomValidMoveForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    //get random moves (teleport/dig) and shoots
    auto movesChar = GetValidTeleportDigs (player1, state, trimStupidMoves);
    std::bitset<8> moves = std::bitset<8>(movesChar);
    std::vector<std::shared_ptr<Command>> shoots = GetShootsForWorm(player1, state, trimStupidMoves);

    //choose random one
    int totalNumMoves = moves.count() + shoots.size();

    if(totalNumMoves == 0) {
        return std::make_shared<DoNothingCommand>();
    }

    std::uniform_int_distribution<int> uniform_dist(0, totalNumMoves-1);
    int mean = uniform_dist(*_rng.get());

    if(mean < static_cast<int>(moves.count())) {
        //get that set bit (TODO there are bit twiddling hacks to do this better)
        int index = -1;
        int numOnesSoFar = 0;
        for(int i = 7; i > -1; --i) {
            ++index;
            if(moves[i]) {
               ++numOnesSoFar;
            }
            if((numOnesSoFar - 1) == mean) {
                break;
            }
        }

        return GetTeleportDig(player1, state, index);

    } else {
        int index = mean - moves.count();
        return shoots[index];
    }
}
