#include "NextTurn.hpp"
#include <algorithm>

std::shared_ptr<pcg32> NextTurn::_rng;
std::vector<std::shared_ptr<Command>> NextTurn::_playerShoots;
std::vector<Position> NextTurn::_surroundingWormSpaces;
std::vector<Position> NextTurn::_relativeBananaTargets;

void NextTurn::Initialise()
{
    if(_playerShoots.empty()) {
        // Seed with a real random value, if available
        pcg_extras::seed_seq_from<std::random_device> seed_source;
        // Make a random number engine 
        _rng = std::make_shared<pcg32>(seed_source);

        //the order of these guys is important (see GetValidShoots)
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NW));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NE));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::W));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::E));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SW));
        _playerShoots.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S));
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

    if(_relativeBananaTargets.empty()) {
        //the order of these guys is important (see GetValidBananas)
        for(int y = (GameConfig::agentWorms.banana.range)*-1; y <= GameConfig::agentWorms.banana.range; ++y) {
            for(int x = (GameConfig::agentWorms.banana.range)*-1; x <= GameConfig::agentWorms.banana.range; ++x) {
                _relativeBananaTargets.push_back(Position(x, y));
            }
        }
    }
}

//returns a bitfield representing the valid directions to move/dig
//bits are set as follows (LSB is bit[0]):
// 0 1 2
// 3 w 4
// 5 6 7
std::bitset<8> NextTurn::GetValidTeleportDigs(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    std::bitset<8> ret(0);
    Worm* worm = player1? state->player1.GetCurrentWorm() : state->player2.GetCurrentWorm();

    for(auto const & space : _surroundingWormSpaces) {
        ret >>= 1;
        Position wormSpace{worm->position + space};
        if(wormSpace.IsOnMap() && state->Cell_at(wormSpace)->worm == nullptr && state->Cell_at(wormSpace)->type != CellType::DEEP_SPACE) {
            ret.set(7);
        }
    }

    return ret;
}

//returns a bitfield representing the valid directions to shoot
//bits are set as follows (LSB is bit[0]):
// 0 1 2
// 3 w 4
// 5 6 7
std::bitset<8> NextTurn::GetValidShoots(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    if (!trimStupidMoves) {
        return std::bitset<8>(255);
    }

    std::bitset<8> ret(0);

    Player* player = player1 ? &state->player1 : &state->player2;

    //TODO can rearrange this so we don't check every direction - rather check where the enemy worms are first
    for(auto const & space : _surroundingWormSpaces) {
        ret >>= 1;
        Worm* hitworm = ShootCommand::WormOnTarget(player1, state, space);
        if(hitworm != nullptr && std::none_of(player->worms.begin(), player->worms.end(), [&](Worm& w){return &w == hitworm;})) {
            ret.set(ret.size() - 1);
        }
    }

    return ret;
}

//returns a bitfield representing the valid targets to lob a banana
//bits are set as follows (LSB is bit[0]):

// 0   1   2   3   4   5   6   7   8   9   10
// 11  12  13  14  15  16  17  18  19  20  21
// 22  23  24  25  26  27  28  29  30  31  32
// 33  34  35  36  37  38  39  40  41  42  43   
// 44  45  46  47  48  49  50  51  52  53  54
// 55  56  57  58  59  {W} 61  62  63  64  65
// 66
// 77
// 88
// 99
// 110 111 112 113 114 115 116 117 118 119 120

//there are 3 bits in each corner that are out of range - they will always be 0
std::bitset<121> NextTurn::GetValidBananas(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    Player* player = player1 ? &state->player1 : &state->player2;
    Worm* worm = player->GetCurrentWorm();

    if(worm->proffession != Worm::Proffession::AGENT || worm->banana_bomb_count <= 0) {
        return std::bitset<121>(0);
    }

    if (!trimStupidMoves) {
        std::bitset<121> ret;
        ret.set();
        return ret;
    }

    std::bitset<121> ret(0);

    for(auto const & space : _relativeBananaTargets) {
        ret >>= 1;

        Position targetPos = worm->position + space;

        if(!targetPos.IsOnMap() || 
            state->Cell_at(targetPos)->type == CellType::DEEP_SPACE ||
            !worm->position.BananaCanReach(targetPos)) {
            continue;
        }

        Worm* hitworm = state->Cell_at(targetPos)->worm;
        if(hitworm != nullptr && std::none_of(player->worms.begin(), player->worms.end(), [&](Worm& w){return &w == hitworm;})) {
            ret.set(ret.size() - 1);
        }

        //TODO get dirt score and consider if above a threshold
        //if we were only considering worms it would be much quicker to just check if any of the opponents are in range
        //dirts could be improved by rather maintaining a bitmap of dirts and XORing

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

std::shared_ptr<Command> NextTurn::GetBanana(bool player1, std::shared_ptr<GameState> state, unsigned index)
{
    Worm* worm = player1? state->player1.GetCurrentWorm() : state->player2.GetCurrentWorm();
    Position targetPos{worm->position + _relativeBananaTargets[index]};

    return std::make_shared<BananaCommand>(targetPos);
}

std::shared_ptr<Command> NextTurn::GetRandomValidMoveForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    //get random moves (teleport/dig) and shoots
    std::bitset<8> moves = std::bitset<8>(GetValidTeleportDigs (player1, state, trimStupidMoves));
    std::bitset<8> shoots = std::bitset<8>(GetValidShoots(player1, state, trimStupidMoves));
    std::bitset<121> bananas = std::bitset<121>(GetValidBananas(player1, state, trimStupidMoves));

    //choose random one
    int totalNumMoves = moves.count() + shoots.count() + bananas.count();

    if(totalNumMoves == 0) {
        return std::make_shared<DoNothingCommand>();
    }

    std::uniform_int_distribution<int> uniform_dist(0, totalNumMoves-1);
    int mean = uniform_dist(*_rng.get());

    if(mean < static_cast<int>(moves.count())) {
        unsigned index = IndexOfIthSetBit(moves, mean);
        return GetTeleportDig(player1, state, index);
    } else if (mean < static_cast<int>(moves.count() + shoots.count())) {
        mean -= moves.count();
        unsigned index = IndexOfIthSetBit(shoots, mean);
        return _playerShoots[index];
    } else {
        mean -= moves.count();
        mean -= shoots.count();
        unsigned index = IndexOfIthSetBit(bananas, mean);
        return GetBanana(player1, state, index);
    }
}

std::vector<std::shared_ptr<Command>> NextTurn::AllValidMovesForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    std::vector<std::shared_ptr<Command>> ret;

    auto moves = NextTurn::GetValidTeleportDigs (player1, state, trimStupidMoves);
    for(unsigned i = 0; i < 8; ++i ) {
        if(moves[i]) {
            ret.push_back(NextTurn::GetTeleportDig(player1, state, i));
        }
    }

    auto possible_shoots = NextTurn::GetValidShoots (player1, state, trimStupidMoves);
    for(unsigned i = 0; i < possible_shoots.size(); ++i ) {
        if(possible_shoots[i]) {
            ret.push_back(NextTurn::_playerShoots[i]);
        }
    }

    std::bitset<121> bananas = std::bitset<121>(GetValidBananas(player1, state, trimStupidMoves));
    for(unsigned i = 0; i < bananas.size(); ++i ) {
        if(bananas[i]) {
            ret.push_back(NextTurn::GetBanana(player1, state, i));
        }
    }

    return ret;
}
