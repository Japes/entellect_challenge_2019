#include "DigCommand.hpp"
#include <iostream>
#include <sstream>

DigCommand::DigCommand(Position pos) : _pos{pos}
{
    _order = 2;
}

//NOTE this assumes move is valid
void DigCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = player1 ? &state->player1 : &state->player2;

    player->consecutiveDoNothingCount = 0;

    state->map[_pos.x][_pos.y].type = CellType::AIR;
    player->command_score += GameConfig::scores.dig;
}

bool DigCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = player1 ? &state->player1 : &state->player2;
    Worm* worm = &player->worms[player->currentWormId-1];

    if (_pos.x >= MAP_SIZE || _pos.y >= MAP_SIZE ||
        _pos.x < 0 || _pos.y < 0 ) {
        std::cerr << "Cant dig off the map..." << _pos << std::endl;
        return false;
    }

    if(state->map[_pos.x][_pos.y].type != CellType::DIRT) {
        std::cerr << "Cant dig air..." << _pos << " (round " << state->roundNumber << " worm " << player->id << worm->id <<  ")" << std::endl;
        return false;
    }

    if (worm->position.MovementDistanceTo(_pos) > worm->diggingRange) {
        std::cerr << _pos << "is too far to dig: " << worm->position.MovementDistanceTo(_pos) << " > " << worm->diggingRange << std::endl;
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
    return _pos == other._pos;
}
