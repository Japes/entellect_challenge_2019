#ifndef TELEPORT_COMMAND_H
#define TELEPORT_COMMAND_H

#include "Command.hpp"

class TeleportCommand : public Command
{
	public:
    TeleportCommand();
    void Execute(bool player1, GameState& state) const override;
};

#endif
