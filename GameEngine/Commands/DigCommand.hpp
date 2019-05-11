#ifndef DIG_COMMAND_H
#define DIG_COMMAND_H

#include "Command.hpp"
#include "Position.hpp"

class DigCommand : public Command
{
	public:
    Position _pos;

    DigCommand(bool player1, std::shared_ptr<GameState> state, Position pos);
    void Execute() const override;
    bool IsValid() const override;
};

#endif
