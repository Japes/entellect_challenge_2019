#ifndef COMMAND_H
#define COMMAND_H

#include "GameState.hpp"

class Command
{
	public:
    virtual void Execute(bool player1, GameState& state) const = 0;
    virtual bool IsValid() const = 0;

    int Order() const
    {
        return _order;
    }

    protected:
    int _order; //the order in which this command should be processed.  Some commands must happen before others

};

#endif
