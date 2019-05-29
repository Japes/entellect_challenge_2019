#include "TeleportCommand.hpp"
#include <iostream>
#include <sstream>

bool TeleportCommand::_randomReturnFlipFlop;

TeleportCommand::TeleportCommand(bool player1, std::shared_ptr<GameState> state, Position pos, bool* forceRandom) :
    Command(player1, state),
    _pos{pos},
    _forceRandom{forceRandom}
{
    _order = 1;
}

//note: assumes move is valid.
void TeleportCommand::Execute() const
{
    Worm* worm_there = _state->map[_pos.x][_pos.y].worm;
    if(worm_there == nullptr) {
        _state->Move_worm(_worm, _pos);
    } else if ( WormMovedThisRound (worm_there)) {

        // 50% chance to pushback or swap positions
        if (FiftyFiftyChance()) {
            //pushback worms
            _state->Move_worm(_worm, _worm->position);

            auto other_worms_previous_pos = worm_there->previous_position;
            _state->Move_worm(worm_there, other_worms_previous_pos);
            _state->Move_worm(worm_there, other_worms_previous_pos); //hacky...so that previous position doesn't show the space he tried to move to
        } else {
            //swap worms
            _state->Move_worm(_worm, worm_there->previous_position);
            _state->Move_worm(worm_there, _worm->previous_position);
        }

        _worm->TakeDamage(GameConfig::pushbackDamage);
        worm_there->TakeDamage(GameConfig::pushbackDamage);
    }

    _player->command_score += GameConfig::scores.move;
}

bool TeleportCommand::IsValid() const
{
    if (_pos.x >= MAP_SIZE || _pos.y >= MAP_SIZE ||
        _pos.x < 0 || _pos.y < 0 ) {
        return false;
    }

    if(_state->map[_pos.x][_pos.y].type != CellType::AIR) {
        return false;
    }

    if (_worm->position.MovementDistanceTo(_pos) > _worm->movementRange) {
        return false;
    }

    Worm* worm_there = _state->map[_pos.x][_pos.y].worm;
    if(worm_there != nullptr && !WormMovedThisRound(worm_there)) {
        return false;
    }

    return true;
}

bool TeleportCommand::WormMovedThisRound(const Worm* worm) const
{
    return worm->previous_position != worm->position;
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
        _player == other._player &&
        _worm == other._worm &&
        _state == other._state &&
        _forceRandom == other._forceRandom &&
        _pos == other._pos;
}

std::string TeleportCommand::GetCommandString() const
{
    std::stringstream ret;
    ret << "move " << _pos.x << " " << _pos.y;
    return ret.str();
}
