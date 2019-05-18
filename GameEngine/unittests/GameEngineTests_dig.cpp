#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"

TEST_CASE( "Dig command", "[Dig_command]" ) {

    GIVEN("A game state and a dig command")
    {
        auto state = std::make_shared<GameState>();

        state->player1.GetCurrentWorm()->position = {10,10};
        state->map[11][10].type = CellType::DIRT;
        state->map[11][11].type = CellType::DIRT;
        state->map[10][11].type = CellType::DEEP_SPACE;
        GameEngine eng(state);

        int expectedDoNothings = 0;
        REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);

        DigCommand player1move(true, state, {0,0});
        DigCommand player2move(false, state, {0,0});

        THEN("out of bounds (too low) is invalid")
        {
            player1move = DigCommand(true, state, {-11,-1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("out of bounds (too high) is invalid")
        {
            player1move = DigCommand(true, state, {200,200});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away is invalid")
        {
            player1move = DigCommand(true, state, {1,1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away is invalid")
        {
            player1move = DigCommand(true, state, {10,12});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("empty space is invalid")
        {
            player1move = DigCommand(true, state, {10,9});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("deep space is invalid")
        {
            player1move = DigCommand(true, state, {10,11});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("digging dirt is valid")
        {
            player1move = DigCommand(true, state, {11,10});    eng.AdvanceState(player1move,player2move);    //++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            REQUIRE(state->map[11][10].type == CellType::AIR);
        }

        THEN("digging dirt diagonally is valid")
        {
            player1move = DigCommand(true, state, {11,11});    eng.AdvanceState(player1move,player2move);    //++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            REQUIRE(state->map[11][11].type == CellType::AIR);
        }
    }
}

//TODO check WormRoundProcessorTest.kt
