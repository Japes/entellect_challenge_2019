#include "DoNothingCommand.hpp"

DoNothingCommand::DoNothingCommand(bool player1, std::shared_ptr<GameState> state) :
    Command(player1, state)
{
    _order = 0; //this is also used to identify a command of this type
}

void DoNothingCommand::Execute() const
{
    //lol
}

bool DoNothingCommand::IsValid() const
{
    return true;
}
