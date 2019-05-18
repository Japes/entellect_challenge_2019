#ifndef DO_NOTHING_COMMAND_H
#define DO_NOTHING_COMMAND_H

#include "Command.hpp"

class DoNothingCommand : public Command
{
	public:
    DoNothingCommand(bool player1, std::shared_ptr<GameState> state);
    void Execute() const override;
    bool IsValid() const override;
};

#endif
