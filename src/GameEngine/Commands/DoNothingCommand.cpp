#include "DoNothingCommand.hpp"

DoNothingCommand::DoNothingCommand()
{
    _order = 0; //this is also used to identify a command of this type
}

void DoNothingCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();
    if(worm->IsFrozen()) {
        return;
    }

    ++player->consecutiveDoNothingCount;
    player->command_score += GameConfig::scores.doNothing;
}

bool DoNothingCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    return true;
}

std::string DoNothingCommand::GetCommandString() const
{
    return "nothing";
}
