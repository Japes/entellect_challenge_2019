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
            INFO("moves: " << moves);
            REQUIRE(moves == 0b10101111);
        }

        THEN("Valid moves for player 2 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(false, state, false);
            INFO("moves: " << moves);
            REQUIRE(moves == 0b11111001);
        }

        AND_THEN("Progressing the game forward 1 turn")
        {
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("Valid moves for player 1 are as expected")
            {
                auto moves = NextTurn::GetValidTeleportDigs(true, state, false);
                INFO("moves: " << moves);
                REQUIRE(moves == 0b11000111);
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
                INFO("moves: " << moves);
                REQUIRE(moves == 0b11111010);
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
        REQUIRE(moves == 0b01100111);

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
        place_worm(true, 3, {20,20}, state);
        place_worm(false, 1, {1,4}, state);
        place_worm(false, 2, {4,4}, state);
        place_worm(false, 3, {30,30}, state);
        state->Cell_at({0, 0})->type = CellType::DIRT;
        state->Cell_at({2, 0})->type = CellType::DIRT;
        state->Cell_at({1, 3})->type = CellType::DIRT;
        state->Cell_at({0, 1})->type = CellType::DEEP_SPACE;

        auto shoots = NextTurn::GetValidShoots(true, state, true);
        INFO("shoots: " << shoots)
        REQUIRE(shoots == 0b10000000);

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
            INFO("shoots: " << ret)
            REQUIRE(ret == 0b10000100);
            REQUIRE(NextTurn::_playerShoots[2]->GetCommandString() == "shoot NE");
            REQUIRE(NextTurn::_playerShoots[7]->GetCommandString() == "shoot SE");            
        }
    }
}

TEST_CASE( "Get sensible bananas", "[get_sensible_bananas]" )
{
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        
        place_worm(true, 3, {31,15}, state); //agent
        place_worm(true, 1, {30,15}, state); //friendly right next to us
        place_worm(true, 2, {0,0}, state); //friendly far away

        place_worm(false, 1, {31,11}, state); //enemy in range to the north
        place_worm(false, 2, {29,13}, state); //enemy in range NW
        place_worm(false, 3, {15,31}, state); //enemy out of range

        WHEN("It's not the agent's turn")
        {
            THEN("GetValidBananas is always 0")
            {
                auto ret = NextTurn::GetValidBananas(true, state, true);
                INFO("shoots: " << ret)
                REQUIRE(ret.count() == 0);
            }
        }

        WHEN("It is the agents turn")
        {
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("GetValidBananas returns correct")
            {
                auto ret = NextTurn::GetValidBananas(true, state, true);
                INFO("shoots: " << ret)
                REQUIRE(ret.count() == 2);
                REQUIRE(ret.test(16));
                REQUIRE(ret.test(36));
                REQUIRE(!ret.test(56)); //returned this when i confused x with y
                REQUIRE(NextTurn::GetBanana(true, state, 16)->GetCommandString() == "banana 31 11");
                REQUIRE(NextTurn::GetBanana(true, state, 36)->GetCommandString() == "banana 29 13");            
            }
        }
    }
}

//just made this to confirm that random moves are actually random
TEST_CASE( "Get random move", "[get_random_move][.statistics]" )
{
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
        place_worm(true, 3, {20,20}, state);
        place_worm(false, 1, {1,4}, state);
        place_worm(false, 2, {4,4}, state);
        place_worm(false, 3, {30,30}, state);
        state->Cell_at({0, 0})->type = CellType::DIRT;
        state->Cell_at({2, 0})->type = CellType::DIRT;
        state->Cell_at({1, 3})->type = CellType::DIRT;
        state->Cell_at({0, 1})->type = CellType::DEEP_SPACE;

        auto cmds = NextTurn::AllValidMovesForPlayer(true, state, true);

        std::vector<int> num_times_this_got_chosen(cmds.size());

        for(unsigned i = 0; i < 10000; ++i) {
            auto cmd =  NextTurn::GetRandomValidMoveForPlayer(true, state, true);

            for(unsigned j = 0; j < cmds.size(); ++j) {
                if(cmd->GetCommandString() == cmds[j]->GetCommandString()) {
                    num_times_this_got_chosen[j] += 1;
                }
            }
        }

        for(unsigned i = 0; i < cmds.size(); ++i) {
            std::cerr << "(" << __FUNCTION__ << ") " << num_times_this_got_chosen[i] << " " << cmds[i]->GetCommandString() << std::endl;
        }

        REQUIRE(false);
    }
}
