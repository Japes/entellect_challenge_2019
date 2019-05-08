#ifndef SHOOT_COMMAND_H
#define SHOOT_COMMAND_H

#include "Command.hpp"

class ShootCommand : public Command
{
	public:
    ShootCommand();
    void Execute(bool player1, GameState& state) const override;
};

#endif
