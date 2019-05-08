#ifndef TELEPORT_COMMAND_H
#define TELEPORT_COMMAND_H

#include "Command.hpp"

class TeleportCommand : public Command
{
	public:
    TeleportCommand();
    void Execute(GameState& state) override;
};

#endif
