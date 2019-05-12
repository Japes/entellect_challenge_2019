#include "GameState.hpp"

GameState::GameState()
{
    player1.id = 1;
    player2.id = 2;
}

Cell* GameState::Cell_at(Position pos)
{
    return &map[pos.x][pos.y];
}

void GameState::Move_worm(Worm* worm, Position pos)
{
    worm->previous_position = worm->position;

    Cell_at(worm->position)->worm = nullptr;
    worm->position = pos;
    Cell_at(worm->position)->worm = worm;
}
