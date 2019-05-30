#ifndef COMMAND_H
#define COMMAND_H

#include "GameState.hpp"
#include <memory>

class Command
{
	public:
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

};

#endif
