#ifndef DIG_COMMAND_H
#define DIG_COMMAND_H

#include "Command.hpp"
#include "Position.hpp"

class DigCommand : public Command
{
	public:
    Position _pos;

    DigCommand(Position pos);
    void Execute(bool player1, std::shared_ptr<GameState> state) const override;
    bool IsValid(bool player1, std::shared_ptr<GameState> state) const override;
    std::string GetCommandString() const override;

    bool operator==(const DigCommand& other);
};


#endif
