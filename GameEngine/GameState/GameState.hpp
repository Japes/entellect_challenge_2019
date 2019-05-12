#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "Player.hpp"
#include "Cell.hpp"
#include "GameConfig.hpp"

//current state of the game

class GameState
{
	public:
    Player player1;
    Player player2;
    Cell map[MAP_SIZE][MAP_SIZE];

    GameState();

    Cell* Cell_at(Position pos);

    void Move_worm(Worm* worm, Position pos);
};

#endif
