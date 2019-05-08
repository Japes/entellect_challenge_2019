#ifndef DO_NOTHING_COMMAND_H
#define DO_NOTHING_COMMAND_H

#include "Command.hpp"

class DoNothingCommand : public Command
{
	public:
    DoNothingCommand();
    void Execute(GameState& state) override;
};

#endif
