#ifndef COMMAND_H
#define COMMAND_H

#include "GameState.hpp"
#include <memory>

class Command
{
	public:
    Command() {}
    Command(bool player1, std::shared_ptr<GameState> state) : _state{state} 
    {
        _player = player1 ? &_state->player1 : &_state->player2;
        _worm = &_player->worms[_player->currentWormId-1];
    }

    int Order() const
    {
        return _order;
    }

    Player* GetPlayer()  const
    {
        return _player;
    }

    virtual void Execute() const = 0;
    virtual bool IsValid() const = 0;

    protected:
    Player* _player; //player this move applies to
    Worm* _worm; //worm this move applies to
    std::shared_ptr<GameState> _state; //reference to the game state
    int _order; //the order in which this command should be processed.  Some commands must happen before others

};

#endif
