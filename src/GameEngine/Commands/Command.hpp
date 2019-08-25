#ifndef COMMAND_H
#define COMMAND_H

#include "GameState.hpp"
#include <memory>
#include <string>

//using GameStatePtr = std::shared_ptr<GameState>;
using GameStatePtr = GameState*;

class Command
{
	public:
    enum class CommandType : uint8_t
    {
        NOTHING = 0,
        SELECT = 1,
        TELEPORT = 2,
        DIG = 3,
        BANANA = 4,
        SHOOT = 5,
        SNOWBALL = 6
    };

    Command() {}

    int Order() const
    {
        return _order;
    }

    virtual void Execute(bool player1, GameStatePtr state) const = 0;
    virtual bool IsValid(bool player1, GameStatePtr state) const = 0;
    virtual std::string GetCommandString() const = 0;

    static const std::string latestBot;
    
    protected:
    int _order; //the order in which this command should be processed.  Some commands must happen before others
};

#endif
