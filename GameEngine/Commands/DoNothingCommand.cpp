#include "DoNothingCommand.hpp"

DoNothingCommand::DoNothingCommand()
{
    _order = 0; //this is also used to identify a command of this type
}

void DoNothingCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = player1 ? &state->player1 : &state->player2;
    ++player->consecutiveDoNothingCount;
}

bool DoNothingCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    return true;
}

std::string DoNothingCommand::GetCommandString() const
{
    return "nothing";
}
