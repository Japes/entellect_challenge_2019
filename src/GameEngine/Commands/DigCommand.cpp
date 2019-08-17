#include "DigCommand.hpp"
#include <iostream>
#include <sstream>

DigCommand::DigCommand(Position pos) : _pos{pos}
{
    _order = static_cast<int>(CommandType::DIG);
}

//NOTE this assumes move is valid
void DigCommand::Execute(bool player1, GameStatePtr state) const
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();
    if(worm->IsFrozen()) {
        return;
    }

    player->consecutiveDoNothingCount = 0;

    state->SetCellTypeAt(_pos, CellType::AIR);
    state->MarkLavaRemovedThisRound(_pos);

    player->command_score += GameConfig::scores.dig;
}

bool DigCommand::IsValid(bool player1, GameStatePtr state) const
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();
    if(worm->IsFrozen()) {
        return true;
    }

    if (_pos.x >= MAP_SIZE || _pos.y >= MAP_SIZE ||
        _pos.x < 0 || _pos.y < 0 ) {
        std::cerr << latestBot << " Cant dig off the map..." << _pos << std::endl;
        return false;
    }

    if(state->CellType_at(_pos) != CellType::DIRT && !state->DirtWasDugThisRound(_pos)) {
        std::cerr << latestBot << " Cant dig air..." << _pos << " (round " << state->roundNumber << " worm " << player->id << worm->id <<  ")" << std::endl;
        return false;
    }

    if (worm->position.MovementDistanceTo(_pos) > worm->diggingRange) {
        std::cerr << latestBot << " " << _pos << "is too far to dig: " << worm->position.MovementDistanceTo(_pos) << " > " << worm->diggingRange << std::endl;
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
