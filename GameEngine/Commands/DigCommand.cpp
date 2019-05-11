#include "DigCommand.hpp"
#include <iostream>

DigCommand::DigCommand(bool player1, std::shared_ptr<GameState> state, Position pos) :
    Command(player1, state),
    _pos{pos}
{
    _order = 1;
}

//NOTE this assumes move is valid
void DigCommand::Execute() const
{
    _state->map[_pos.x][_pos.y].type = CellType::AIR;
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
        std::cout << "Too far: " << _worm->position.MovementDistanceTo(_pos) << " > " << _worm->diggingRange << std::endl;
        return false;
    }

    return true;
}
