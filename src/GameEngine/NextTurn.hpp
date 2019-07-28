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
    static std::vector<Position> _relativeBananaTargets;
    
    static void Initialise();

    static std::bitset<8> GetValidTeleportDigs(Worm* worm, std::shared_ptr<GameState> state, bool trimStupidMoves);
    static std::shared_ptr<Command> GetTeleportDig(Worm* worm, std::shared_ptr<GameState> state, unsigned index);
    static std::bitset<8> GetValidShoots(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);
    static std::bitset<121> GetBananaMiningTargets(Worm* worm, std::shared_ptr<GameState> state, int thresh);
    static std::bitset<121> GetValidBananas(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);
    static std::shared_ptr<Command> GetBanana(Worm* worm, std::shared_ptr<GameState> state, unsigned index);


    static std::shared_ptr<Command> GetRandomValidMoveForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);    
    static std::vector<std::shared_ptr<Command>> AllValidMovesForPlayer(bool player1, std::shared_ptr<GameState> state, bool trimStupidMoves);
    
    //heuristic related stuff
    static std::shared_ptr<Command> GetNearestDirtHeuristic(bool player1, std::shared_ptr<GameState> state, int distanceForLost);
    static std::shared_ptr<Command> GetBananaProspect(bool player1, std::shared_ptr<GameState> state, int thresh);
    static std::string TryApplySelect(bool player1, std::shared_ptr<GameState> state);

    private:
    static std::shared_ptr<pcg32> _rng;

    static std::vector<Position> _surroundingWormSpaces;

    template <class T>
    static unsigned IndexOfIthSetBit(T bits, unsigned i)
    {
        //get that set bit (TODO there are bit twiddling hacks to do this better)
        int index = -1;
        unsigned numOnesSoFar = 0;
        for(size_t j = 0; j < bits.size(); ++j) {
            ++index;
            if(bits[j]) {
                ++numOnesSoFar;
            }
            if((numOnesSoFar - 1) == i) {
                break;
            }
        }

        return index;
    }

};

#endif
