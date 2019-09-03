#include "catch.hpp"
#include "GameConfig.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Handle no available moves", "[no_available_moves]" )
{
    GIVEN("A game state where the worm has no valid moves")
    {
        GameState state;
        
        place_worm(true, 1, {10,10}, state);
        state.SetCellTypeAt({9, 9}, CellType::DEEP_SPACE);
        state.SetCellTypeAt({9, 10}, CellType::DEEP_SPACE);
        state.SetCellTypeAt({9, 11}, CellType::DEEP_SPACE);
        state.SetCellTypeAt({10, 9}, CellType::DEEP_SPACE);
        state.SetCellTypeAt({11, 9}, CellType::DEEP_SPACE);
        state.SetCellTypeAt({11, 10}, CellType::DEEP_SPACE);
        state.SetCellTypeAt({11, 11}, CellType::DEEP_SPACE);
        state.SetCellTypeAt({10, 11}, CellType::DEEP_SPACE);


        THEN("We return the donothing command (if we trim stupid shoots)")
        {
            auto ret = NextTurn::GetRandomValidMoveForPlayer(true, &state, true);
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

        GameState state;
        GameEngine eng(&state);
        Worm* worm11 = place_worm(true, 1, {5,5}, state);
        Worm* worm12 = place_worm(true, 2, {6,5}, state);
        place_worm(true, 3, {3,3}, state);
        Worm* worm21 = place_worm(false, 1, {5,6}, state);
        place_worm(false, 2, {4,1}, state);
        place_worm(false, 3, {5,3}, state);
        state.SetCellTypeAt({5, 4}, CellType::DIRT);
        state.SetCellTypeAt({4, 5}, CellType::DIRT);
        state.SetCellTypeAt({4, 6}, CellType::DIRT);
        state.SetCellTypeAt({7, 5}, CellType::DEEP_SPACE);

        THEN("Valid moves for worm 11 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(worm11, &state, false);
            INFO("moves: " << moves);
            REQUIRE(moves == 0b10101111);
        }

        THEN("Valid moves for worm 21 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(worm21, &state, false);
            INFO("moves: " << moves);
            REQUIRE(moves == 0b11111001);
        }

        THEN("Valid moves for worm 12 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(worm12, &state, false);
            INFO("moves: " << moves);
            REQUIRE(moves == 0b11000111);
        }

        AND_THEN("Moving a dude, and cycling to his turn again")
        {
            eng.AdvanceState(TeleportCommand({4,4}), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("Valid moves for player 1 are as expected")
            {
                auto moves = NextTurn::GetValidTeleportDigs(worm11, &state, false);
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
        2   L   .   22
        */

        GameState state;
        Worm* worm11 = place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {2,1}, state);
        place_worm(false, 2, {2,2}, state);
        state.SetCellTypeAt({0, 0}, CellType::DIRT);
        state.SetCellTypeAt({2, 0}, CellType::DIRT);
        state.SetCellTypeAt({0, 1}, CellType::DEEP_SPACE);
        state.AddLavaAt({0, 2});

        auto moves = NextTurn::GetValidTeleportDigs(worm11, &state, false);
        REQUIRE(moves == 0b01100111);

        REQUIRE(NextTurn::GetTeleportDig(worm11, &state, 0)->GetCommandString() == "dig 0 0");
        REQUIRE(NextTurn::GetTeleportDig(worm11, &state, 1)->GetCommandString() == "move 1 0");
        REQUIRE(NextTurn::GetTeleportDig(worm11, &state, 2)->GetCommandString() == "dig 2 0");
        REQUIRE(NextTurn::GetTeleportDig(worm11, &state, 3)->GetCommandString() == "nothing"); //error case...
        REQUIRE(NextTurn::GetTeleportDig(worm11, &state, 4)->GetCommandString() == "nothing"); //error case...
        REQUIRE(NextTurn::GetTeleportDig(worm11, &state, 5)->GetCommandString() == "move 0 2");
        REQUIRE(NextTurn::GetTeleportDig(worm11, &state, 6)->GetCommandString() == "move 1 2");
        REQUIRE(NextTurn::GetTeleportDig(worm11, &state, 7)->GetCommandString() == "nothing"); //error case...
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

        GameState state;
        place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {2,1}, state);
        place_worm(true, 3, {20,20}, state);
        place_worm(false, 1, {1,4}, state);
        place_worm(false, 2, {4,4}, state);
        place_worm(false, 3, {30,30}, state);
        state.SetCellTypeAt({0, 0}, CellType::DIRT);
        state.SetCellTypeAt({2, 0}, CellType::DIRT);
        state.SetCellTypeAt({1, 3}, CellType::DIRT);
        state.SetCellTypeAt({0, 1}, CellType::DEEP_SPACE);

        auto shoots = NextTurn::GetValidShoots(true, &state, true, false);
        INFO("shoots: " << shoots)
        REQUIRE(shoots == 0b10000000);

        REQUIRE(NextTurn::_playerShoots[7]->GetCommandString() == "shoot SE");
    }
}

TEST_CASE( "Get sensible shoots", "[get_sensible_shoots]" )
{

    /*
        0   1   2   3   4   5   6   7   8   9   10
    0   .   .   .   .   .   .   .   .   .   .   .
    1   .   .   .   21  .   .   .   .   .   .   .
    2   13  .   11  12  .   .   .   .   .   .   .
    3   .   .   .   .   .   .   .   .   .   .   .
    4   .   .   .   .   .   .   .   .   .   .   .
    5   .   .   .   .   .   22  .   .   .   .   .
    6   .   .   .   .   .   .   .   .   .   .   .
    7   .   .   23  .   .   .   .   .   .   .   .
    8   .   .   .   .   .   .   .   .   .   .   .
    9   .   .   .   .   .   .   .   .   .   .   .
    10  .   .   .   .   .   .   .   .   .   .   .
    */

    GIVEN("A semi realistic game state and engine")
    {
        GameState state;
        
        bool player1 = GENERATE(true, false);
        place_worm(player1, 1, {2,2}, state);
        place_worm(player1, 2, {3,2}, state); //friendly E of us
        place_worm(player1, 3, {0,2}, state); //friendly W of us
        place_worm(!player1, 1, {3,1}, state); //enemy NE 1 step
        place_worm(!player1, 2, {5,5}, state); //enemy SE 2 step
        place_worm(!player1, 3, {2,7}, state); //just out of range

        THEN("GetValidShoots returns correct")
        {
            auto ret = NextTurn::GetValidShoots(player1, &state, true, false);
            INFO("shoots: " << ret)
            REQUIRE(ret == 0b10000100);
            REQUIRE(NextTurn::_playerShoots[2]->GetCommandString() == "shoot NE");
            REQUIRE(NextTurn::_playerShoots[7]->GetCommandString() == "shoot SE");            
        }
    }
}

TEST_CASE( "Get sensible shoots - blocking", "[get_sensible_shoots_blocking]" )
{

    /*
        0   1   2   3   4   5   6   7   8   9   10
    0   .   .   .   .   .   .   .   .   .   .   .
    1   .   .   .   .   .   .   .   .   .   .   .
    2   13  .   11  12  .   21  .   .   .   .   .
    3   .   .   .   .   .   .   .   .   .   .   .
    4   .   .   .   .   D   .   .   .   .   .   .
    5   .   .   .   .   .   22  .   .   .   .   .
    6   .   .   23  .   .   .   .   .   .   .   .
    7   .   .   .   .   .   .   .   .   .   .   .
    8   .   .   .   .   .   .   .   .   .   .   .
    9   .   .   .   .   .   .   .   .   .   .   .
    10  .   .   .   .   .   .   .   .   .   .   .
    */

    GIVEN("A semi realistic game state and engine")
    {
        GameState state;
        
        bool player1 = GENERATE(true, false);
        place_worm(player1, 1, {2,2}, state);
        place_worm(player1, 2, {3,2}, state);
        place_worm(player1, 3, {0,2}, state);
        place_worm(!player1, 1, {5,2}, state);
        place_worm(!player1, 2, {5,5}, state);
        place_worm(!player1, 3, {2,6}, state);
        state.SetCellTypeAt({4, 4}, CellType::DIRT);

        THEN("GetValidShoots returns correct")
        {
            auto ret = NextTurn::GetValidShoots(player1, &state, true, false);
            INFO("shoots: " << ret)
            REQUIRE(ret == 0b01000000);
            REQUIRE(NextTurn::_playerShoots[6]->GetCommandString() == "shoot S");            
        }
    }
}

TEST_CASE( "Get sensible shoots one-off", "[get_sensible_shoots]" )
{
    /*
        0   1   2   3   4   5   6   7   8   9   10
    0   .   .   .   .   .   .   .   .   .   .   .
    1   .   .   .   21  .   .   .   .   .   .   .
    2   13  .   11  12  .   .   .   .   .   .   .
    3   .   .   .   .   .   .   .   .   .   .   .
    4   .   .   .   .   D   .   .   .   .   .   .
    5   .   .   .   .   .   .   22  .   .   .   .
    6   .   .   .   .   .   .   .   .   .   .   .
    7   .   .   23  .   .   .   .   .   .   .   .
    8   .   .   .   .   .   .   .   .   .   .   .
    9   .   .   .   .   .   .   .   .   .   .   .
    10  .   .   .   .   .   .   .   .   .   .   .
    */

    GIVEN("A semi realistic game state and engine")
    {
        GameState state;
        
        bool player1 = GENERATE(true, false);
        place_worm(player1, 1, {2,2}, state);
        place_worm(player1, 2, {3,2}, state);
        place_worm(player1, 3, {0,2}, state);
        place_worm(!player1, 1, {3,1}, state);
        place_worm(!player1, 2, {6,5}, state);
        place_worm(!player1, 3, {2,7}, state);
        state.SetCellTypeAt({4, 4}, CellType::DIRT);

        THEN("GetValidShoots returns correct")
        {
            auto ret = NextTurn::GetValidShoots(player1, &state, true, true);
            INFO("shoots: " << ret)
            REQUIRE(ret == 0b00000110); //S, NE, N
        }
    }

}

TEST_CASE( "Get sensible shoots one-off ... correct worm only", "[get_sensible_shoots_correct_worm]" )
{
    /*
        0   1   2   3   4   
    0   .   .   .   .   .   
    1   .   .  (12)(D)  .   
    2   .   22  11  21  .
    3   .   .  (S) (L)  .   
    4   .   .   .   .   .   
    */

    GIVEN("A semi realistic game state and engine")
    {
        GameState state;
        
        bool player1 = GENERATE(true, false);
        place_worm(player1, 1, {2,2}, state);
        auto rightEnemy = place_worm(!player1, 1, {3,2}, state);
        auto leftEnemy = place_worm(!player1, 2, {1,2}, state);

        place_worm(player1, 3, {21,20}, state);
        place_worm(!player1, 3, {22,20}, state);

        THEN("GetValidShoots only considers that currentWorm could move")
        {
            auto ret = NextTurn::GetValidShoots(player1, &state, true, true);
            REQUIRE(ret == 0b11011110); //E, NE, SE
        }

        THEN("GetValidShoots understands that frozen worms can't move")
        {
            rightEnemy->roundsUntilUnfrozen = 10;
            leftEnemy->roundsUntilUnfrozen = 10;
            auto ret = NextTurn::GetValidShoots(player1, &state, true, true);
            REQUIRE(ret == 0b00011000);
        }

        THEN("GetValidShoots understands rules about movement")
        {
            place_worm(player1, 2, {2,1}, state);
            state.SetCellTypeAt({3, 1}, CellType::DIRT);
            state.SetCellTypeAt({2, 3}, CellType::DEEP_SPACE);
            state.AddLavaAt({3, 3});
            auto ret = NextTurn::GetValidShoots(player1, &state, true, true);
            REQUIRE(ret == 0b10011000);
        }
    }
    
}

TEST_CASE( "Get sensible snowballs", "[get_sensible_snowballs]" )
{
    GIVEN("A semi realistic game state and engine")
    {
        GameState state;
        GameEngine eng(&state);
        
        Worm* worm13 = place_worm(true, 3, {31,15}, state); //technologist
        place_worm(true, 1, {30,15}, state); //friendly right next to us
        place_worm(true, 2, {0,0}, state); //friendly far away

        place_worm(false, 1, {31,11}, state); //enemy in range to the north
        auto dedguy = place_worm(false, 2, {29,13}, state); //enemy in range NW
        dedguy->health = -1;
        place_worm(false, 3, {15,31}, state); //enemy out of range

        WHEN("It's not the techies turn")
        {
            THEN("GetValidSnowballs is always 0")
            {
                auto ret = NextTurn::GetValidSnowballs(true, &state, true);
                INFO("shoots: " << ret)
                REQUIRE(ret.count() == 0);
            }
        }

        WHEN("It is the techies turn")
        {
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            AND_WHEN("He has snowballs")
            {
                AND_WHEN("The heuristic blocks him ")
                {
                    worm13->health = 60;
                    state.roundNumber = 300;
                    THEN("GetValidSnowballs returns zero")
                    {
                        auto ret = NextTurn::GetValidSnowballs(true, &state, true);
                        INFO("shoots: " << ret)
                        REQUIRE(ret.count() == 0);
                    }
                }

                AND_WHEN("The heuristic doesn't block him (health)")
                {
                    worm13->health = 49;
                    state.roundNumber = 7;
                    THEN("GetValidSnowballs returns correct")
                    {
                        auto ret = NextTurn::GetValidSnowballs(true, &state, true);
                        INFO("shoots: " << ret)
                        CHECK(ret.count() == 1);
                        CHECK(ret.test(16));
                        CHECK(!ret.test(36)); //dedguy
                        CHECK(!ret.test(56)); //returned this when i confused x with y
                        CHECK(NextTurn::GetSnowball(worm13, 16)->GetCommandString() == "snowball 31 11");          
                    }
                }

                AND_WHEN("The heuristic doesn't block him (round)")
                {
                    worm13->health = 100;
                    state.roundNumber = 390;
                    THEN("GetValidSnowballs returns correct")
                    {
                        auto ret = NextTurn::GetValidSnowballs(true, &state, true);
                        INFO("shoots: " << ret)
                        CHECK(ret.count() == 1);
                        CHECK(ret.test(16));
                        CHECK(!ret.test(36)); //dedguy
                        CHECK(!ret.test(56)); //returned this when i confused x with y
                        CHECK(NextTurn::GetSnowball(worm13, 16)->GetCommandString() == "snowball 31 11");          
                    }
                }
            }
            AND_WHEN("He has no snowballs")
            {
                state.player1.worms[2].snowball_count = 0;
                THEN("GetValidSnowballs returns zero")
                {
                    auto ret = NextTurn::GetValidSnowballs(true, &state, true);
                    INFO("shoots: " << ret)
                    REQUIRE(ret.count() == 0);
                }
            }
        }

        //test heuristics here 
        //bool shouldHoldOnToSnowBall = (worm->health > 50) && (state->roundNumber < GameConfig::maxRounds - 75);
        //if(enemyWorm.roundsUntilUnfrozen > 1) {

    }
}

TEST_CASE( "Get sensible bananas", "[get_sensible_bananas]" )
{
    GIVEN("A semi realistic game state and engine")
    {
        GameState state;
        GameEngine eng(&state);
        
        Worm* worm12 = place_worm(true, 2, {31,15}, state); //agent
        place_worm(true, 1, {30,15}, state); //friendly right next to us
        place_worm(true, 3, {0,0}, state); //friendly far away

        place_worm(false, 1, {31,11}, state); //enemy in range to the north
        auto dedguy = place_worm(false, 2, {29,13}, state); //enemy in range NW
        dedguy->health = -1;
        place_worm(false, 3, {15,31}, state); //enemy out of range

        WHEN("It's not the agent's turn")
        {
            THEN("GetValidBananas is always 0")
            {
                auto ret = NextTurn::GetValidBananas(true, &state, true);
                INFO("shoots: " << ret)
                REQUIRE(ret.count() == 0);
            }
        }

        WHEN("It is the agents turn")
        {
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            AND_WHEN("He has bananas")
            {
                THEN("GetValidBananas returns correct")
                {
                    auto ret = NextTurn::GetValidBananas(true, &state, true);
                    INFO("shoots: " << ret)
                    REQUIRE(ret.count() == 1);
                    REQUIRE(ret.test(16));
                    REQUIRE(!ret.test(36)); //dedguy
                    REQUIRE(!ret.test(56)); //returned this when i confused x with y
                    REQUIRE(NextTurn::GetBanana(worm12, 16)->GetCommandString() == "banana 31 11");          
                }
            }
            AND_WHEN("He has no bananas")
            {
                state.player1.worms[1].banana_bomb_count = 0;
                THEN("GetValidBananas returns zero")
                {
                    auto ret = NextTurn::GetValidBananas(true, &state, true);
                    INFO("shoots: " << ret)
                    REQUIRE(ret.count() == 0);
                }
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

        GameState state;
        place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {2,1}, state);
        place_worm(true, 3, {20,20}, state);
        place_worm(false, 1, {1,4}, state);
        place_worm(false, 2, {4,4}, state);
        place_worm(false, 3, {30,30}, state);
        state.SetCellTypeAt({0, 0}, CellType::DIRT);
        state.SetCellTypeAt({2, 0}, CellType::DIRT);
        state.SetCellTypeAt({1, 3}, CellType::DIRT);
        state.SetCellTypeAt({0, 1}, CellType::DEEP_SPACE);

        auto cmds = NextTurn::AllValidMovesForPlayer(true, &state, true);

        std::vector<int> num_times_this_got_chosen(cmds.size());

        for(unsigned i = 0; i < 10000; ++i) {
            auto cmd =  NextTurn::GetRandomValidMoveForPlayer(true, &state, true);

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
