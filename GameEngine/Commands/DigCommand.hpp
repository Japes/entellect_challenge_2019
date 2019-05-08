#ifndef DIG_COMMAND_H
#define DIG_COMMAND_H

#include "Command.hpp"

class DigCommand : public Command
{
	public:
    DigCommand();
    void Execute(bool player1, GameState& state) const override;
};

#endif
