#ifndef BANANA_COMMAND_H
#define BANANA_COMMAND_H

#include "Command.hpp"

class BananaCommand : public Command
{
	public:
    Position _pos;

    BananaCommand(Position pos);
    void Execute(bool player1, GameStatePtr state) const override;
    bool IsValid(bool player1, GameStatePtr state) const override;
    std::string GetCommandString() const override;
    
    bool operator==(const BananaCommand& other);

    private:
    static std::vector<std::vector<int>> _splashDamageLookup;
};

#endif
