#include "GameEngineTestUtils.hpp"
#include <fstream>
#include <sstream>

Worm* place_worm(bool player1, int wormNumber, Position pos, std::shared_ptr<GameState> state)
{
    return place_worm(player1, wormNumber, pos, *state.get());
}

Worm* place_worm(bool player1, int wormNumber, Position pos, GameState& state)
{
    Worm* worm_under_test;
    if(player1) {
        worm_under_test = &state.player1.worms[wormNumber - 1];
    } else {
        worm_under_test = &state.player2.worms[wormNumber - 1];
    }
    
    state.PlaceWormAt(pos, worm_under_test);

    return worm_under_test;
};

void place_powerup(Position pos, GameState& state)
{
    state.PlacePowerupAt(pos);
};
