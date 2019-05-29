#include "DigCommand.hpp"
#include <iostream>
#include <sstream>

DigCommand::DigCommand(bool player1, std::shared_ptr<GameState> state, Position pos) :
    Command(player1, state),
    _pos{pos}
{
    _order = 2;
}

//NOTE this assumes move is valid
void DigCommand::Execute() const
{
    _state->map[_pos.x][_pos.y].type = CellType::AIR;
    _player->command_score += GameConfig::scores.dig;
}

bool DigCommand::IsValid() const
{
    if (_pos.x >= MAP_SIZE || _pos.y >= MAP_SIZE ||
        _pos.x < 0 || _pos.y < 0 ) {
        return false;
    }

    if(_state->map[_pos.x][_pos.y].type != CellType::DIRT) {
        return false;
    }

    if (_worm->position.MovementDistanceTo(_pos) > _worm->diggingRange) {
        std::cerr << "Too far: " << _worm->position.MovementDistanceTo(_pos) << " > " << _worm->diggingRange << std::endl;
        return false;
    }

    return true;
}

std::string DigCommand::GetCommandString() const
{
    std::stringstream ret;
    ret << "dig " << _pos.x << " " << _pos.y;
    return ret.str();
}

bool DigCommand::operator==(const DigCommand& other)
{
        return
        _player == other._player &&
        _worm == other._worm &&
        _state == other._state &&
        _pos == other._pos;
}
