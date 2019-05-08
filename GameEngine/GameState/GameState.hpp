#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "Player.hpp"
#include "Cell.hpp"

//current state of the game

class GameState
{
	public:
    Player player1;
    Player player2;
    Cell map[33][33];

    GameState();

};

#endif
