#ifndef NEXT_TURN_H
#define NEXT_TURN_H

#include "GameState.hpp"
#include "AllCommands.hpp"
#include <vector>
#include <random>
#include <functional>
#include <bitset>
#include "pcg_random.hpp"

class NextTurn
{
	public:
    static std::vector<std::shared_ptr<Command>> _playerShoots;
    
    static void Initialise();

    static std::bitset<8> GetValidTeleportDigs(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);
    static std::shared_ptr<Command> GetTeleportDig(bool player1, std::shared_ptr<GameState> state, unsigned index);
    static std::bitset<8> GetValidShoots(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);

    static std::shared_ptr<Command> GetRandomValidMoveForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);    
    static std::vector<std::shared_ptr<Command>> AllValidMovesForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);
    
    private:
    static std::shared_ptr<pcg32> _rng;

    static std::vector<Position> _surroundingWormSpaces;

    static unsigned IndexOfIthSetBit(std::bitset<8> bits, unsigned i);

};

#endif
