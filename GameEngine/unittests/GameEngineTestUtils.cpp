#include "GameEngineTestUtils.hpp"
#include <fstream>
#include <sstream>

Worm* place_worm(bool player1, int wormNumber, Position pos, std::shared_ptr<GameState> state)
{
    Worm* worm_under_test;
    if(player1) {
        worm_under_test = &state->player1.worms[wormNumber - 1];
    } else {
        worm_under_test = &state->player2.worms[wormNumber - 1];
    }
    worm_under_test->position = worm_under_test->previous_position = pos;
    state->Cell_at(pos)->worm = worm_under_test;

    return worm_under_test;
};

void place_powerup(Position pos, std::shared_ptr<GameState> state)
{
    static int powerupIndex = 0;
    state->PlacePowerupAt(pos, powerupIndex);
    ++powerupIndex;
};
