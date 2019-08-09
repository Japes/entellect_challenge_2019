#include "SelectCommand.hpp"
#include <iostream>
#include <sstream>

SelectCommand::SelectCommand(int wormIndex, std::shared_ptr<Command> selectedCmd) :
    _wormIndex{wormIndex},
    _selectedCmd{selectedCmd}
{
    _order = 1;
}

//note: assumes move is valid.
void SelectCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = state->GetPlayer(player1);
    player->currentWormId = _wormIndex;
    --player->remainingWormSelections;
    _selectedCmd->Execute(player1, state);
}

bool SelectCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    if(_wormIndex < 1 || _wormIndex > 3) {
        return false;
    }

    Player* player = state->GetPlayer(player1);
    auto wormIDBefore = player->currentWormId;

    player->currentWormId = _wormIndex;
    bool valid = _selectedCmd->IsValid(player1, state);
    player->currentWormId = wormIDBefore;

    return valid;
}

bool SelectCommand::operator==(const SelectCommand& other)
{
    return
    _wormIndex == other._wormIndex &&
    _selectedCmd == other._selectedCmd;
}

std::string SelectCommand::GetCommandString() const
{
    std::stringstream ret;
    ret << "select " << _wormIndex << ";" << _selectedCmd->GetCommandString();
    return ret.str();
}
