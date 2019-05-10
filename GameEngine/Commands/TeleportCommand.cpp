#include "TeleportCommand.hpp"

TeleportCommand::TeleportCommand(Position pos) :
    _pos{pos}
{
    _order = 2;
}

void TeleportCommand::Execute(bool player1, GameState& state) const
{

}

bool TeleportCommand::IsValid() const
{
    return true;
}
