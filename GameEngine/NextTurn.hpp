#ifndef NEXT_TURN_H
#define NEXT_TURN_H

#include "GameState.hpp"
#include "AllCommands.hpp"
#include <vector>
#include <random>
#include <functional>
#include "pcg_random.hpp"

class NextTurn
{
	public:
    static std::vector<std::shared_ptr<Command>> _playerShoots;
    
    static void Initialise();

    static uint8_t GetValidTeleportDigs(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);
    static std::shared_ptr<Command> GetTeleportDig(bool player1, std::shared_ptr<GameState> state, unsigned index);

    static std::shared_ptr<Command> GetRandomValidMoveForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);
    static std::vector<std::shared_ptr<Command>> GetShootsForWorm(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);
    
    
    private:
    static std::shared_ptr<pcg32> _rng;

    static std::vector<Position> _surroundingWormSpaces;

};

#endif
