#ifndef TELEPORT_COMMAND_H
#define TELEPORT_COMMAND_H

#include "Command.hpp"

class TeleportCommand : public Command
{
	public:
    Position _pos;

    TeleportCommand(bool player1, std::shared_ptr<GameState> state, Position pos);
    void Execute() const override;
    bool IsValid() const override;
};

#endif
