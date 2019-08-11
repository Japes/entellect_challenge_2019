#ifndef COMMAND_H
#define COMMAND_H

#include "GameState.hpp"
#include <memory>
#include <string>

class Command
{
	public:
    enum class CommandType : uint8_t
    {
        NOTHING = 0,
        SELECT = 1,
        TELEPORT = 2,
        DIG = 3,
        SNOWBALL = 4, //temporary, apparently snowball will become last
        BANANA = 5,
        SHOOT = 6
    };

    Command() {}

    int Order() const
    {
        return _order;
    }

    virtual void Execute(bool player1, std::shared_ptr<GameState> state) const = 0;
    virtual bool IsValid(bool player1, std::shared_ptr<GameState> state) const = 0;
    virtual std::string GetCommandString() const = 0;

    protected:
    int _order; //the order in which this command should be processed.  Some commands must happen before others
    static const std::string latestBot;
};

#endif
