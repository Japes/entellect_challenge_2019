#ifndef SNOWBALL_COMMAND_H
#define SNOWBALL_COMMAND_H

#include "Command.hpp"

class SnowballCommand : public Command
{
	public:
    Position _pos;

    SnowballCommand(Position pos);
    void Execute(bool player1, std::shared_ptr<GameState> state) const override;
    bool IsValid(bool player1, std::shared_ptr<GameState> state) const override;
    std::string GetCommandString() const override;
    
    bool operator==(const SnowballCommand& other);
};

#endif
