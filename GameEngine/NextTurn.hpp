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
    static void Initialise();

    static std::vector<std::shared_ptr<Command>> GetValidTeleportDigsForWorm(bool player1, 
                                                    std::shared_ptr<GameState> state, bool trimStupidMoves = false);
    static std::shared_ptr<Command> GetRandomValidMoveForWorm(bool player1, 
                                                    std::shared_ptr<GameState> state, bool trimStupidMoves = false);
    static std::vector<std::shared_ptr<Command>> GetSensibleShootsForWorm(bool player1, 
                                                    std::shared_ptr<GameState> state);
    static std::vector<std::shared_ptr<Command>> _playerShoots;
    
    private:
    static std::shared_ptr<pcg32> _rng;

    static std::vector<Position> _surroundingWormSpaces;

};

#endif
