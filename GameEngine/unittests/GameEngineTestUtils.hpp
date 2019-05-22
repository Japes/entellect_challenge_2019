#ifndef GAME_ENGINE_UTILS_H
#define GAME_ENGINE_UTILS_H

#include "Worm.hpp"
#include "GameState.hpp"
#include <memory>

Worm* place_worm(bool player1, int wormNumber, Position pos, std::shared_ptr<GameState> state);
void place_powerup(Position pos, std::shared_ptr<GameState> state);

#endif