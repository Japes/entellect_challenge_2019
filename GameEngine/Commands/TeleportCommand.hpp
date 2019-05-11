#ifndef TELEPORT_COMMAND_H
#define TELEPORT_COMMAND_H

#include "Command.hpp"

class TeleportCommand : public Command
{
	public:
    Position _pos;

    TeleportCommand(Position pos);
    void Execute() const override;
    bool IsValid() const override;
};

#endif
