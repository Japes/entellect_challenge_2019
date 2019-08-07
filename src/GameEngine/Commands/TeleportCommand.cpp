#include "TeleportCommand.hpp"
#include <iostream>
#include <sstream>

bool TeleportCommand::_randomReturnFlipFlop;

TeleportCommand::TeleportCommand(Position pos, bool* forceRandom) :
    _pos{pos},
    _forceRandom{forceRandom}
{
    _order = 1;
}

//note: assumes move is valid.
void TeleportCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();

    player->consecutiveDoNothingCount = 0;
    
    Worm* worm_there = state->Worm_at(_pos);
    if(worm_there == nullptr) {
        state->Move_worm(worm, _pos);
    } else if ( worm_there->movedThisRound ) {

        // 50% chance to pushback or swap positions
        if (FiftyFiftyChance()) {
            //pushback worms
            state->Move_worm(worm, worm->position);

            auto other_worms_previous_pos = worm_there->previous_position;
            state->Move_worm(worm_there, other_worms_previous_pos);
            state->Move_worm(worm_there, other_worms_previous_pos); //hacky...so that previous position doesn't show the space he tried to move to
        } else {
            //swap worms
            state->Move_worm(worm, worm_there->previous_position);
            state->Move_worm(worm_there, worm->previous_position);
        }

        worm->TakeDamage(GameConfig::pushbackDamage);
        worm_there->TakeDamage(GameConfig::pushbackDamage);

        //both lost points for taking damage
        state->player1.command_score -= GameConfig::pushbackDamage*2;
        state->player2.command_score -= GameConfig::pushbackDamage*2;

    }

    player->command_score += GameConfig::scores.move;
    worm->movedThisRound = true;
}

bool TeleportCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();

    if (_pos.x >= MAP_SIZE || _pos.y >= MAP_SIZE ||
        _pos.x < 0 || _pos.y < 0 ) {
        std::cerr << latestBot << "------Cant move off the map..." << _pos << std::endl;
        return false;
    }

    if(state->CellType_at(_pos) == CellType::DIRT || state->CellType_at(_pos) == CellType::DEEP_SPACE) {
        std::cerr << latestBot << "------Cant move through dirt or deep-space..." << _pos << std::endl;
        return false;
    }

    if (worm->position.MovementDistanceTo(_pos) > worm->movementRange) {
        std::cerr << latestBot << " " << _pos << "------is too far to move: " << worm->position.MovementDistanceTo(_pos) << " > " << worm->movementRange << ". Worm is at pos " << worm->position << std::endl;
        return false;
    }

    Worm* worm_there = state->Worm_at(_pos);
    if(worm_there != nullptr && !worm_there->movedThisRound) {
        std::cerr << latestBot << "------Cant move into space " << _pos << ", occupied by worm " << worm_there->id << " (" << worm_there << ").  I am worm " << worm->id << "(" << worm << ")" << std::endl;
        return false;
    }

    return true;
}

//returns true 50% of the time, false otherwise
bool TeleportCommand::FiftyFiftyChance() const
{
    if(_forceRandom != nullptr) {
        return *_forceRandom;
    }

    _randomReturnFlipFlop = ~_randomReturnFlipFlop;
    return _randomReturnFlipFlop;
}

bool TeleportCommand::operator==(const TeleportCommand& other)
{
        return
        _forceRandom == other._forceRandom &&
        _pos == other._pos;
}

std::string TeleportCommand::GetCommandString() const
{
    std::stringstream ret;
    ret << "move " << _pos.x << " " << _pos.y;
    return ret.str();
}
