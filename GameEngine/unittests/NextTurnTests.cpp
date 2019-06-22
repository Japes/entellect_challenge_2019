#include "catch.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Handle no available moves", "[no_available_moves]" )
{
    GIVEN("A game state where the worm has no valid moves")
    {
        auto state = std::make_shared<GameState>();
        
        place_worm(true, 1, {10,10}, state);
        state->Cell_at({9, 9})->type = CellType::DEEP_SPACE;
        state->Cell_at({9, 10})->type = CellType::DEEP_SPACE;
        state->Cell_at({9, 11})->type = CellType::DEEP_SPACE;
        state->Cell_at({10, 9})->type = CellType::DEEP_SPACE;
        state->Cell_at({11, 9})->type = CellType::DEEP_SPACE;
        state->Cell_at({11, 10})->type = CellType::DEEP_SPACE;
        state->Cell_at({11, 11})->type = CellType::DEEP_SPACE;
        state->Cell_at({10, 11})->type = CellType::DEEP_SPACE;


        THEN("We return the donothing command (if we trim stupid shoots)")
        {
            auto ret = NextTurn::GetRandomValidMoveForPlayer(true, state, true);
            REQUIRE(ret->GetCommandString() == "nothing");
        }
    }
}

bool Contains_one(std::vector<std::shared_ptr<Command>>& haystack, std::shared_ptr<Command> needle)
{
    int num_found = 0;
    for(unsigned i = 0; i < haystack.size(); i++) {

        {
            ShootCommand* hay_ptr = dynamic_cast<ShootCommand*>(haystack[i].get());
            ShootCommand* needle_ptr = dynamic_cast<ShootCommand*>(needle.get());
            if(hay_ptr != nullptr && needle_ptr != nullptr &&
                *hay_ptr == *needle_ptr) {
                num_found++;
            }
        }

        {
            TeleportCommand* hay_ptr = dynamic_cast<TeleportCommand*>(haystack[i].get());
            TeleportCommand* needle_ptr = dynamic_cast<TeleportCommand*>(needle.get());
            if(hay_ptr != nullptr && needle_ptr != nullptr &&
                *hay_ptr == *needle_ptr) {
                num_found++;
            }
        }

        {
            DigCommand* hay_ptr = dynamic_cast<DigCommand*>(haystack[i].get());
            DigCommand* needle_ptr = dynamic_cast<DigCommand*>(needle.get());
            if(hay_ptr != nullptr && needle_ptr != nullptr &&
                *hay_ptr == *needle_ptr) {
                num_found++;
            }
        }
    }

    return num_found == 1;
}

TEST_CASE( "GetValidTeleportDigs", "[GetValidTeleportDigs]" ) {
    GIVEN("A semi realistic game state and engine")
    {
        /*
            0   1   2   3   4   5   6   7
        0   .   .   .   .   .   .   .   .
        1   .   .   .   .   22  .   .   .
        2   .   .   .   .   .   .   .   .
        3   .   .   .   13  .   23  .   .
        4   .   .   .   .   .   D   .   .
        5   .   .   .   .   D   11  12  S
        6   .   .   .   .   D   21  .   .
        7   .   .   .   .   .   .   .   .            
        */

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {5,5}, state);
        place_worm(true, 2, {6,5}, state);
        place_worm(true, 3, {3,3}, state);
        place_worm(false, 1, {5,6}, state);
        place_worm(false, 2, {4,1}, state);
        place_worm(false, 3, {5,3}, state);
        state->Cell_at({5, 4})->type = CellType::DIRT;
        state->Cell_at({4, 5})->type = CellType::DIRT;
        state->Cell_at({4, 6})->type = CellType::DIRT;
        state->Cell_at({7, 5})->type = CellType::DEEP_SPACE;

        THEN("Valid moves for player 1 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(true, state, false);
            INFO("moves: " << (int)moves);
            REQUIRE(moves == 0b11110101);
        }

        THEN("Valid moves for player 2 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(false, state, false);
            INFO("moves: " << (int)moves);
            REQUIRE(moves == 0b10011111);
        }

        AND_THEN("Progressing the game forward 1 turn")
        {
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("Valid moves for player 1 are as expected")
            {
                auto moves = NextTurn::GetValidTeleportDigs(true, state, false);
                INFO("moves: " << (int)moves);
                REQUIRE(moves == 0b11100011);
            }
        }

        AND_THEN("Moving a dude, and cycling to his turn again")
        {
            eng.AdvanceState(TeleportCommand({4,4}), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("Valid moves for player 1 are as expected")
            {
                auto moves = NextTurn::GetValidTeleportDigs(true, state, false);
                INFO("moves: " << (int)moves);
                REQUIRE(moves == 0b01011111);
            }
        }
    }
}

TEST_CASE( "GetTeleportDig", "[GetTeleportDig]" ) {
    GIVEN("A semi realistic game state")
    {

        /*
            0   1   2
        0   D   .   D
        1   S   11  12
        2   .   .   22
        */

        auto state = std::make_shared<GameState>();
        place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {2,1}, state);
        place_worm(false, 2, {2,2}, state);
        state->Cell_at({0, 0})->type = CellType::DIRT;
        state->Cell_at({2, 0})->type = CellType::DIRT;
        state->Cell_at({0, 1})->type = CellType::DEEP_SPACE;

        auto moves = NextTurn::GetValidTeleportDigs(true, state, false);
        REQUIRE(moves == 0b11100110);

        REQUIRE(NextTurn::GetTeleportDig(true, state, 0)->GetCommandString() == "dig 0 0");
        REQUIRE(NextTurn::GetTeleportDig(true, state, 1)->GetCommandString() == "move 1 0");
        REQUIRE(NextTurn::GetTeleportDig(true, state, 2)->GetCommandString() == "dig 2 0");
        REQUIRE(NextTurn::GetTeleportDig(true, state, 3)->GetCommandString() == "nothing"); //error case...
        REQUIRE(NextTurn::GetTeleportDig(true, state, 4)->GetCommandString() == "nothing"); //error case...
        REQUIRE(NextTurn::GetTeleportDig(true, state, 5)->GetCommandString() == "move 0 2"); //error case...
        REQUIRE(NextTurn::GetTeleportDig(true, state, 6)->GetCommandString() == "move 1 2");
        REQUIRE(NextTurn::GetTeleportDig(true, state, 7)->GetCommandString() == "nothing"); //error case...
    }
}

TEST_CASE( "GetValidShoots", "[GetValidShoots]" ) {
    GIVEN("A semi realistic game state")
    {

        /*
            0   1   2   3   4
        0   D   .   D   .   .
        1   S   11  12  .   .
        2   .   .   .   .   .   
        3   .   D   .   .   .
        4   .   21  .   .   22
        */

        auto state = std::make_shared<GameState>();
        place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {2,1}, state);
        place_worm(true, 2, {20,20}, state);
        place_worm(false, 1, {1,4}, state);
        place_worm(false, 2, {4,4}, state);
        place_worm(false, 2, {30,30}, state);
        state->Cell_at({0, 0})->type = CellType::DIRT;
        state->Cell_at({2, 0})->type = CellType::DIRT;
        state->Cell_at({1, 3})->type = CellType::DIRT;

        auto shoots = NextTurn::GetValidShoots(true, state, true);
        INFO("shoots: " << (int)shoots)
        REQUIRE(shoots == 0b00000001);

        REQUIRE(NextTurn::_playerShoots[7]->GetCommandString() == "shoot SE");
    }
}

TEST_CASE( "Get sensible shoots", "[get_sensible_shoots]" )
{
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        
        place_worm(true, 1, {10,10}, state);
        place_worm(true, 2, {11,10}, state); //friendly E of us
        place_worm(true, 3, {8,10}, state); //friendly W of us
        place_worm(false, 1, {11,9}, state); //enemy NE 1 step
        place_worm(false, 2, {13,13}, state); //enemy SE 2 step
        place_worm(false, 3, {10,15}, state); //just out of range

        THEN("GetValidShoots returns correct")
        {
            auto ret = NextTurn::GetValidShoots(true, state, true);
            INFO("shoots: " << (int)ret)
            REQUIRE(ret == 0b00100001);
            REQUIRE(NextTurn::_playerShoots[2]->GetCommandString() == "shoot NE");
            REQUIRE(NextTurn::_playerShoots[7]->GetCommandString() == "shoot SE");            
        }
    }
}
