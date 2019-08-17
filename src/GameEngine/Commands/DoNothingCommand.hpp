#ifndef DO_NOTHING_COMMAND_H
#define DO_NOTHING_COMMAND_H

#include "Command.hpp"

class DoNothingCommand : public Command
{
	public:
    DoNothingCommand();
    void Execute(bool player1, GameStatePtr state) const override;
    bool IsValid(bool player1, GameStatePtr state) const override;
    std::string GetCommandString() const override;
};

#endif
