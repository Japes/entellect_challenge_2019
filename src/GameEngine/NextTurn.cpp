#include "NextTurn.hpp"
#include "GameEngine.hpp"
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
std::bitset<8> NextTurn::GetValidTeleportDigs(Worm* worm, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    std::bitset<8> ret(0);

    for(auto const & space : _surroundingWormSpaces) {
        ret >>= 1;
        Position wormSpace{worm->position + space};
        if(wormSpace.IsOnMap() && state->CellType_at(wormSpace) != CellType::DEEP_SPACE && state->Worm_at(wormSpace) == nullptr ) {
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

    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();

    auto enemyWorms = player1? &(state->player2.worms) : &(state->player1.worms);

    Position noShot{0,0};
    for(auto const & enemyWorm : (*enemyWorms)) {
        Position shootVec = ShootCommand::GetValidShot(*worm, enemyWorm, state);
        if(shootVec != noShot) {
            auto it = std::find(_surroundingWormSpaces.begin(), _surroundingWormSpaces.end(), shootVec);
            if (it != _surroundingWormSpaces.end()) {
                int index = std::distance(_surroundingWormSpaces.begin(), it);
                ret.set(index);
            }

        }
    }

    return ret;
}

std::shared_ptr<Command> NextTurn::GetBananaProspect(bool player1, std::shared_ptr<GameState> state, int thresh)
{
    Worm* worm = player1? state->player1.GetCurrentWorm() : state->player2.GetCurrentWorm();

    std::bitset<121> bananas = std::bitset<121>(GetBananaMiningTargets(worm, state, thresh));

    //choose random one
    int totalNumMoves = bananas.count();
    if(totalNumMoves == 0) {
        return nullptr;
    }

    std::uniform_int_distribution<int> uniform_dist(0, totalNumMoves-1);
    int mean = uniform_dist(*_rng.get());

    unsigned index = IndexOfIthSetBit(bananas, mean);
    return GetBanana(worm, state, index);
}

//returns banana moves that will hit at least [thresh] dirts
//uses same format as GetValidBananas
std::bitset<121> NextTurn::GetBananaMiningTargets(Worm* worm, std::shared_ptr<GameState> state, int thresh)
{
    if(worm->proffession != Worm::Proffession::AGENT || worm->banana_bomb_count <= 0) {
        return std::bitset<121>(0);
    }

    std::bitset<121> ret; //TODO not strictly correct - includes corners

    Position startPos = worm->position - Position(GameConfig::agentWorms.banana.range, GameConfig::agentWorms.banana.range);
    startPos = startPos - Position(1,1);
    Position endPos = worm->position + Position(GameConfig::agentWorms.banana.range, GameConfig::agentWorms.banana.range);
    Position target = startPos;

    //std::cerr << "(" << __FUNCTION__ << ") startPos: " << startPos << " endPos " << endPos << std::endl;

    int retIndex = -1;
    while(target.y < endPos.y) {
        ++target.y;
        while(target.x < endPos.x) {
            ++target.x;
            ++retIndex;
            //std::cerr << "(" << __FUNCTION__ << ") target " << target << " retIndex " << retIndex << std::endl;
            if(!worm->position.BananaCanReach(target)) {
                //std::cerr << "(" << __FUNCTION__ << ") no thanks... " << std::endl;
                continue;
            }

            if(state->DirtsBananaWillHit(target) >= thresh) {
                //std::cerr << "(" << __FUNCTION__ << ") cool setting bit " << retIndex << " position is " << target << std::endl;
                ret.set(retIndex);
            }
        }
        target.x = startPos.x;
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
// 66  67  68  69  70  71  72  73  74
// 77  78  79  80  81  82  83  84  85
// 88
// 99
// 110 111 112 113 114 115 116 117 118 119 120

//there are 3 bits in each corner that are out of range - they will always be 0
std::bitset<121> NextTurn::GetValidBananas(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();

    if(worm->proffession != Worm::Proffession::AGENT || worm->banana_bomb_count <= 0) {
        return std::bitset<121>(0);
    }

    if (!trimStupidMoves) {
        std::bitset<121> ret; //TODO not strictly correct - includes corners
        ret.set();
        return ret;
    }

    std::bitset<121> ret(0);

    auto enemyWorms = player1? &(state->player2.worms) : &(state->player1.worms);

    for(auto const & enemyWorm : (*enemyWorms)) {
        if(worm->position.BananaCanReach(enemyWorm.position)) {
            Position posDiff = enemyWorm.position - worm->position;
            int index = 60 + posDiff.x + (posDiff.y*11); //see ascii art at top of this function
            ret.set(index);
        }
    }

    return ret;
}

std::shared_ptr<Command> NextTurn::GetTeleportDig(Worm* worm, std::shared_ptr<GameState> state, unsigned index)
{
    Position targetPos{worm->position + _surroundingWormSpaces[index]};

    if(state->Worm_at(targetPos) == nullptr) {
        CellType t = state->CellType_at(targetPos);
        if(t == CellType::AIR) {
            return std::make_shared<TeleportCommand>(targetPos);
        } else if(t == CellType::DIRT) {
            return std::make_shared<DigCommand>(targetPos);
        }
    }

    std::cerr << "(" << __FUNCTION__ << ") index specified that isn't a valid teleport or dig.  Doing nothing." << std::endl;
    return std::make_shared<DoNothingCommand>();
}

std::shared_ptr<Command> NextTurn::GetBanana(Worm* worm, std::shared_ptr<GameState> state, unsigned index)
{
    Position targetPos{worm->position + _relativeBananaTargets[index]};

    return std::make_shared<BananaCommand>(targetPos);
}

std::shared_ptr<Command> NextTurn::GetRandomValidMoveForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    //get random moves (teleport/dig) and shoots
    Worm* worm = player1? state->player1.GetCurrentWorm() : state->player2.GetCurrentWorm();

    std::bitset<8> moves = std::bitset<8>(GetValidTeleportDigs (worm, state, trimStupidMoves));
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
        return GetTeleportDig(worm, state, index);
    } else if (mean < static_cast<int>(moves.count() + shoots.count())) {
        mean -= moves.count();
        unsigned index = IndexOfIthSetBit(shoots, mean);
        return _playerShoots[index];
    } else {
        mean -= moves.count();
        mean -= shoots.count();
        unsigned index = IndexOfIthSetBit(bananas, mean);
        return GetBanana(worm, state, index);
    }
}

std::vector<std::shared_ptr<Command>> NextTurn::AllValidMovesForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves)
{
    std::vector<std::shared_ptr<Command>> ret;
    Worm* worm = player1? state->player1.GetCurrentWorm() : state->player2.GetCurrentWorm();

    auto moves = NextTurn::GetValidTeleportDigs (worm, state, trimStupidMoves);
    for(unsigned i = 0; i < 8; ++i ) {
        if(moves[i]) {
            ret.push_back(NextTurn::GetTeleportDig(worm, state, i));
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
            ret.push_back(NextTurn::GetBanana(worm, state, i));
        }
    }

    return ret;
}

//checks if a select should happen (based on a heuristic...)
//if so, progresses gamestate until it's that worm's turn, and returns a non-empty string with the "select" command that should be applied.
std::string NextTurn::TryApplySelect(bool player1, std::shared_ptr<GameState> state)
{
    if(GetValidShoots(player1, state, true).any()) {
        return "";
    }

    auto myState = std::make_shared<GameState>(*state); //no idea why it needs to be done this way

    Player* player = myState->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();
    if(player->remainingWormSelections <= 0) {
        return "";
    }

    GameEngine myEng(myState);
    myEng.AdvanceState(DoNothingCommand(), DoNothingCommand());
    int numAdvancesApplied = 1;

    while(player->GetCurrentWorm() != worm) {

        if(GetValidShoots(player1, myState, true).any()) {
            //cool we have a candidate.  project the given state forward so the caller can use it
            GameEngine eng(state);
            for(int i = 0; i < numAdvancesApplied; ++i) {
                eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
            }
            return (std::string("select ") + std::to_string(player->GetCurrentWorm()->id) + ";");
        }

        myEng.AdvanceState(DoNothingCommand(), DoNothingCommand());
        ++numAdvancesApplied;
    }

    return "";
}