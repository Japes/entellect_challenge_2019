#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"

TEST_CASE( "I can make a game engine instance", "[sanity]" ) {
    GameEngine eng;
    REQUIRE( true );
}

TEST_CASE( "Commands are resolved in the right order", "[command_order]" ) {
    //according to the rules:
    //move
    //dig
    //shoot
}

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

TEST_CASE( "Move command validation", "[Move_command_validation]" ) {

    GIVEN("A game state and a move command")
    {
        auto state = std::make_shared<GameState>();

        /*
            the situation (worm under test is in the middle):
            [9,9]W2   [10,9] W1  [11,9] A
            [9,10]A   [10,10]W   [11,10]D
            [9,11]A   [10,11]S   [11,11]D
        */

        Position worm_under_test_pos{10,10};
        Worm* worm_under_test = &state->player1.worms[0];
        worm_under_test->position = worm_under_test->previous_position = worm_under_test_pos;
        state->Cell_at(worm_under_test_pos)->worm = worm_under_test;

        Position friendly_worm_pos{10,9};
        Worm* friendly_worm = &state->player1.worms[1];
        friendly_worm->position = friendly_worm->previous_position = friendly_worm_pos;
        state->Cell_at(friendly_worm_pos)->worm = friendly_worm;

        Position enemy_worm_pos{9,9};
        Worm* enemy_worm = &state->player2.worms[1];
        enemy_worm->position =  enemy_worm->previous_position = enemy_worm_pos;
        state->Cell_at(enemy_worm_pos)->worm = enemy_worm;

        Position dirt_pos_straight{11,10};
        state->Cell_at(dirt_pos_straight)->type = CellType::DIRT;

        Position dirt_pos_diag{11,11};
        state->Cell_at(dirt_pos_diag)->type = CellType::DIRT;

        Position deep_space_pos{10,11};
        state->Cell_at(deep_space_pos)->type = CellType::DEEP_SPACE;

        GameEngine eng(state);

        int expectedDoNothings = 0;
        REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);

        TeleportCommand player1move(true, state, {0,0});
        TeleportCommand player2move(false, state, {0,0});

        THEN("out of bounds (too low) is invalid")
        {
            player1move = TeleportCommand(true, state, {-1,-1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("out of bounds (too high) is invalid")
        {
            player1move = TeleportCommand(true, state, {200,200});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far (really far) away is invalid")
        {
            player1move = TeleportCommand(true, state, {1,1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away (just out of range) is invalid")
        {
            player1move = TeleportCommand(true, state, {10,12});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away is invalid")
        {
            player1move = TeleportCommand(true, state, {8,8});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("dirt is invalid")
        {
            player1move = TeleportCommand(true, state, dirt_pos_straight);    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("dirt (diag) is invalid")
        {
            player1move = TeleportCommand(true, state, dirt_pos_diag);    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("deep space is invalid")
        {
            player1move = TeleportCommand(true, state, deep_space_pos);    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("Spot with a friendly worm is invalid")
        {
            player1move = TeleportCommand(true, state, friendly_worm_pos);    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("Spot with an enemy worm is invalid")
        {
            player1move = TeleportCommand(true, state, enemy_worm_pos);    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }
    }
}

TEST_CASE( "Move command execution", "[Move_command_execution]" ) {

    GIVEN("A game state and a move command")
    {
        auto state = std::make_shared<GameState>();

        /*
            the situation (worm under test is in the middle):
            [9,9]W2   [10,9] A   [11,9] A
            [9,10]A   [10,10]W   [11,10]A
            [9,11]A   [10,11]A   [11,11]A
        */

        Position worm_under_test_pos{10,10};
        Worm* worm_under_test = &state->player1.worms[0];
        worm_under_test->position = worm_under_test->previous_position = worm_under_test_pos;
        state->Cell_at(worm_under_test_pos)->worm = worm_under_test;

        Position enemy_worm_pos{9,9};
        Worm* enemy_worm = &state->player2.worms[0];
        enemy_worm->position =  enemy_worm->previous_position = enemy_worm_pos;
        state->Cell_at(enemy_worm_pos)->worm = enemy_worm;

        Position air_pos1{11,9};
        Position air_pos2{11,10};
        Position air_pos3{9,10};

        GameEngine eng(state);

        int expectedDoNothings = 0;
        REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);

        TeleportCommand player1move(true, state, {0,0});
        TeleportCommand player2move(false, state, {0,0});

        THEN("air is valid1")
        {
            player1move = TeleportCommand(true, state, air_pos1);    eng.AdvanceState(player1move,player2move);
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            bool happy = state->player1.GetWormByIndex(1)->position == air_pos1;
            REQUIRE(happy);
        }

        THEN("air is valid2")
        {
            player1move = TeleportCommand(true, state, air_pos2);    eng.AdvanceState(player1move,player2move);
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            bool happy = state->player1.GetWormByIndex(1)->position == air_pos2;
            REQUIRE(happy);
        }

        THEN("air is valid3")
        {
            player1move = TeleportCommand(true, state, air_pos3);    eng.AdvanceState(player1move,player2move);
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            bool happy = state->player1.GetWormByIndex(1)->position == air_pos3;
            REQUIRE(happy);
        }

        THEN("Worm collision works - pushback")
        {
            auto healthBefore1 = state->player1.GetWormByIndex(1)->health;
            auto healthBefore2 = state->player2.GetWormByIndex(1)->health;

            bool forcePushback = true;
            player1move = TeleportCommand(true, state, air_pos3, &forcePushback);
            player2move = TeleportCommand(false, state, air_pos3, &forcePushback);
            eng.AdvanceState(player1move,player2move);

            CHECK(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            bool happy = state->player1.GetWormByIndex(1)->position == worm_under_test_pos;
            CHECK(happy);
            happy = state->player2.GetWormByIndex(1)->position == enemy_worm_pos;
            CHECK(happy);

            CHECK(state->player1.GetWormByIndex(1)->health == healthBefore1 - GameConfig::pushbackDamage);
            CHECK(state->player1.GetWormByIndex(1)->health == healthBefore2 - GameConfig::pushbackDamage);
        }

        THEN("Worm collision works - swap")
        {
            auto healthBefore1 = state->player1.GetWormByIndex(1)->health;
            auto healthBefore2 = state->player2.GetWormByIndex(1)->health;

            bool forcePushback = false; //force swap
            player1move = TeleportCommand(true, state, air_pos3, &forcePushback);
            player2move = TeleportCommand(false, state, air_pos3, &forcePushback);
            eng.AdvanceState(player1move,player2move);

            CHECK(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            bool happy = state->player1.GetWormByIndex(1)->position == enemy_worm_pos;
            CHECK(happy);
            happy = state->player2.GetWormByIndex(1)->position == worm_under_test_pos;
            CHECK(happy);

            CHECK(state->player1.GetWormByIndex(1)->health == healthBefore1 - GameConfig::pushbackDamage);
            CHECK(state->player2.GetWormByIndex(1)->health == healthBefore2 - GameConfig::pushbackDamage);
        }

        //test that when a worm moves, the space he leaves is empty


        //what if you move into a space that a worm is vacating this round????

        
    }
}

//check all command types

TEST_CASE( "Active worms are chosen correctly", "[active_worm]" ) {
    //test when worms have died
}

TEST_CASE( "12 do nothings means disqualified", "[disqualified]" ) {
    //check for player 1 and player 2
}

TEST_CASE( "Points are allocated correctly", "[command_order]" ) {
    //check for each thing mentioned in the rules and confirm game engine concurs
}

TEST_CASE( "Comparison with jave engine", "[comparison]" ) {
    //read a bunch of known state files, moves, and expected outputs generated by their engine.
    //read in those files, apply the moves, and compare result with theirs.
}
