#ifndef COMMAND_H
#define COMMAND_H

#include "GameState.hpp"

class Command
{
	public:
    virtual void Execute(GameState& state) = 0;

    protected:
    int _order; //the order in which this command should be processed.  Some commands must happen before others

};

#endif
