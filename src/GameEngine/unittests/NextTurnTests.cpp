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
        state->SetCellTypeAt({9, 9}, CellType::DEEP_SPACE);
        state->SetCellTypeAt({9, 10}, CellType::DEEP_SPACE);
        state->SetCellTypeAt({9, 11}, CellType::DEEP_SPACE);
        state->SetCellTypeAt({10, 9}, CellType::DEEP_SPACE);
        state->SetCellTypeAt({11, 9}, CellType::DEEP_SPACE);
        state->SetCellTypeAt({11, 10}, CellType::DEEP_SPACE);
        state->SetCellTypeAt({11, 11}, CellType::DEEP_SPACE);
        state->SetCellTypeAt({10, 11}, CellType::DEEP_SPACE);


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
        Worm* worm11 = place_worm(true, 1, {5,5}, state);
        Worm* worm12 = place_worm(true, 2, {6,5}, state);
        place_worm(true, 3, {3,3}, state);
        Worm* worm21 = place_worm(false, 1, {5,6}, state);
        place_worm(false, 2, {4,1}, state);
        place_worm(false, 3, {5,3}, state);
        state->SetCellTypeAt({5, 4}, CellType::DIRT);
        state->SetCellTypeAt({4, 5}, CellType::DIRT);
        state->SetCellTypeAt({4, 6}, CellType::DIRT);
        state->SetCellTypeAt({7, 5}, CellType::DEEP_SPACE);

        THEN("Valid moves for worm 11 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(worm11, state, false);
            INFO("moves: " << moves);
            REQUIRE(moves == 0b10101111);
        }

        THEN("Valid moves for worm 21 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(worm21, state, false);
            INFO("moves: " << moves);
            REQUIRE(moves == 0b11111001);
        }

        THEN("Valid moves for worm 12 are as expected")
        {
            auto moves = NextTurn::GetValidTeleportDigs(worm12, state, false);
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
                auto moves = NextTurn::GetValidTeleportDigs(worm11, state, false);
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
        Worm* worm11 = place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {2,1}, state);
        place_worm(false, 2, {2,2}, state);
        state->SetCellTypeAt({0, 0}, CellType::DIRT);
        state->SetCellTypeAt({2, 0}, CellType::DIRT);
        state->SetCellTypeAt({0, 1}, CellType::DEEP_SPACE);

        auto moves = NextTurn::GetValidTeleportDigs(worm11, state, false);
        REQUIRE(moves == 0b01100111);

        REQUIRE(NextTurn::GetTeleportDig(worm11, state, 0)->GetCommandString() == "dig 0 0");
        REQUIRE(NextTurn::GetTeleportDig(worm11, state, 1)->GetCommandString() == "move 1 0");
        REQUIRE(NextTurn::GetTeleportDig(worm11, state, 2)->GetCommandString() == "dig 2 0");
        REQUIRE(NextTurn::GetTeleportDig(worm11, state, 3)->GetCommandString() == "nothing"); //error case...
        REQUIRE(NextTurn::GetTeleportDig(worm11, state, 4)->GetCommandString() == "nothing"); //error case...
        REQUIRE(NextTurn::GetTeleportDig(worm11, state, 5)->GetCommandString() == "move 0 2"); //error case...
        REQUIRE(NextTurn::GetTeleportDig(worm11, state, 6)->GetCommandString() == "move 1 2");
        REQUIRE(NextTurn::GetTeleportDig(worm11, state, 7)->GetCommandString() == "nothing"); //error case...
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
        state->SetCellTypeAt({0, 0}, CellType::DIRT);
        state->SetCellTypeAt({2, 0}, CellType::DIRT);
        state->SetCellTypeAt({1, 3}, CellType::DIRT);
        state->SetCellTypeAt({0, 1}, CellType::DEEP_SPACE);

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
        
        Worm* worm12 = place_worm(true, 2, {31,15}, state); //agent
        place_worm(true, 1, {30,15}, state); //friendly right next to us
        place_worm(true, 3, {0,0}, state); //friendly far away

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

            AND_WHEN("He has bananas")
            {
                THEN("GetValidBananas returns correct")
                {
                    auto ret = NextTurn::GetValidBananas(true, state, true);
                    INFO("shoots: " << ret)
                    REQUIRE(ret.count() == 2);
                    REQUIRE(ret.test(16));
                    REQUIRE(ret.test(36));
                    REQUIRE(!ret.test(56)); //returned this when i confused x with y
                    REQUIRE(NextTurn::GetBanana(worm12, state, 16)->GetCommandString() == "banana 31 11");
                    REQUIRE(NextTurn::GetBanana(worm12, state, 36)->GetCommandString() == "banana 29 13");            
                }
            }
            AND_WHEN("He has no bananas")
            {
                state->player1.worms[1].banana_bomb_count = 0;
                THEN("GetValidBananas returns zero")
                {
                    auto ret = NextTurn::GetValidBananas(true, state, true);
                    INFO("shoots: " << ret)
                    REQUIRE(ret.count() == 0);
                }
            }
        }
    }
}

TEST_CASE( "GetBananaMiningTargets", "[GetBananaMiningTargets]" )
{
    GIVEN("A semi realistic game state")
    {
        /* stars show the extent of a worms reach
        0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17
        0   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
        1   .   .   .   .   .   *   *   *   *   D   *   *   .   .   .   .   .
        2   .   .   .   .   *   .   .   .   .   .   .   .   *   .   .   .   .
        3   .   .   .   *   .   .   .   D   .   .   .   D   .   *   .   .   .
        4   .   .   .   *   .   D   .   .   .   .   .   .   .   *   .   .   .
        5   .   .   .   *   D   D   D   .   .   D   .   .   .   *   .   .   .
        6   .   .   .   *D  D   D   D   D   W   .   .   .   .   *   .   .   .
        7   .   .   .   *   D   D   D   .   .   .   .   .   .   *   .   .   .    
        8   .   .   .   *   .   D   .   .   .   .   .   .   .   *   .   .   .    
        9   .   .   .   *   .   .   .   .   D   .   .   .   .   *   .   .   .    
        10  .   .   .   .   *   .   .   D   D   D   .   .   *   .   .   .   .    
        11  .   .   .   .   .   *   *D  *D  *  *D  *D  *   .   .   .   .   .    
        12  .   .   .   .   .   .   .   D   D   D   .   .   .   .   .   .   .
        13  .   .   .   .   .   .   .   .   D   .   .   .   .   .   .   .   .
        14  .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .   .
        */

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        auto thrower = place_worm(true, 2, {9,6}, state);
        //make it the agent's turn...
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

        //big clump to the W
        state->SetCellTypeAt({6, 4}, CellType::DIRT);
        state->SetCellTypeAt({5, 5}, CellType::DIRT); state->SetCellTypeAt({6, 5}, CellType::DIRT); state->SetCellTypeAt({7, 5}, CellType::DIRT);
        state->SetCellTypeAt({4, 6}, CellType::DIRT); state->SetCellTypeAt({5, 6}, CellType::DIRT); state->SetCellTypeAt({6, 6}, CellType::DIRT); state->SetCellTypeAt({7, 6}, CellType::DIRT); state->SetCellTypeAt({8, 6}, CellType::DIRT);
        state->SetCellTypeAt({5, 7}, CellType::DIRT); state->SetCellTypeAt({6, 7}, CellType::DIRT); state->SetCellTypeAt({7, 7}, CellType::DIRT);
        state->SetCellTypeAt({6, 8}, CellType::DIRT);

        //sparse four to the NE
        state->SetCellTypeAt({10, 1}, CellType::DIRT); state->SetCellTypeAt({8, 3}, CellType::DIRT);
        state->SetCellTypeAt({12, 3}, CellType::DIRT); state->SetCellTypeAt({10, 5}, CellType::DIRT);

        //big clump to the S
        state->SetCellTypeAt({9, 9}, CellType::DIRT);
        state->SetCellTypeAt({8, 10}, CellType::DIRT); state->SetCellTypeAt({9, 10}, CellType::DIRT); state->SetCellTypeAt({10, 10}, CellType::DIRT);
        state->SetCellTypeAt({7, 11}, CellType::DIRT); state->SetCellTypeAt({8, 11}, CellType::DIRT); state->SetCellTypeAt({10, 11}, CellType::DIRT); state->SetCellTypeAt({11, 11}, CellType::DIRT);
        state->SetCellTypeAt({8, 12}, CellType::DIRT); state->SetCellTypeAt({9, 12}, CellType::DIRT); state->SetCellTypeAt({10, 12}, CellType::DIRT);
        state->SetCellTypeAt({9, 13}, CellType::DIRT);

        WHEN("He has bananas")
        {
            THEN("GetBananaMiningTargets returns correct")
            {
                auto ret = NextTurn::GetBananaMiningTargets(thrower, state, 13);
                INFO("bananas: " << ret)
                REQUIRE(ret.count() == 1);
                REQUIRE(ret.test(57));
                REQUIRE(NextTurn::GetBanana(thrower, state, 57)->GetCommandString() == "banana 6 6");

            }

            THEN("GetBananaMiningTargets returns correct")
            {
                auto ret = NextTurn::GetBananaMiningTargets(thrower, state, 12);
                INFO("bananas: " << ret)
                REQUIRE(ret.count() == 2);
                REQUIRE(ret.test(57));
                REQUIRE(NextTurn::GetBanana(thrower, state, 57)->GetCommandString() == "banana 6 6");
                REQUIRE(ret.test(115));
                REQUIRE(NextTurn::GetBanana(thrower, state, 115)->GetCommandString() == "banana 9 11");            
            }

            THEN("GetBananaMiningTargets returns correct")
            {
                auto ret = NextTurn::GetBananaMiningTargets(thrower, state, 4);
                INFO("bananas: " << ret)
                REQUIRE(ret.test(28));
                REQUIRE(NextTurn::GetBanana(thrower, state, 28)->GetCommandString() == "banana 10 3");            
                REQUIRE(ret.count() > 6); //can see at LEAST 6 in that ascii picture
            }

            THEN("Banana prospecting returns correct")
            {
                auto bananaCommand = NextTurn::GetBananaProspect(true, state, 12);
                REQUIRE(bananaCommand != nullptr);
                bool happy = bananaCommand->GetCommandString() == "banana 6 6" || bananaCommand->GetCommandString() == "banana 9 11";
                REQUIRE(happy);
            }

            THEN("Banana prospecting returns correct")
            {
                auto bananaCommand = NextTurn::GetBananaProspect(true, state, 4);
                REQUIRE(bananaCommand != nullptr);
            }
        }

        WHEN("He has no bananas")
        {
            thrower->banana_bomb_count = 0;
            THEN("GetBananaMiningTargets returns zero")
            {
                auto ret = NextTurn::GetBananaMiningTargets(thrower, state, 1);
                REQUIRE(ret.count() == 0);
            }

            THEN("Banana prospecting returns correct")
            {
                auto bananaCommand = NextTurn::GetBananaProspect(true, state, 13);
                REQUIRE(bananaCommand == nullptr);
            }
        }
    }
}

TEST_CASE( "Heuristic to avoid getting lost", "[GetNearestDirtHeuristic]" )
{
    GIVEN("A contrived game state")
    {

        //    0   1   2   3   4 
        //0   .   .   .   .   .
        //1   .   11  .   .   .
        //2   .   .   .   .   .
        //3   .   .   .   .   .
        //4   .   .   .   .   .

        auto state = std::make_shared<GameState>();

        bool player1 = GENERATE(true, false);
        int distanceForLost = GENERATE(2, 5, 10, 25);

        Worm* playerWorm = place_worm(player1, 1, {1,1}, state);

        WHEN("there is an enemy worm within the range limit")
        {
            auto enemyWormPos = playerWorm->position + Position(distanceForLost - 1, distanceForLost/2);
            place_worm(player1, 1, enemyWormPos, state);

            THEN("the heuristic doesn't kick in")
            {
                auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
                REQUIRE(cmd == nullptr);
            }
        }

        WHEN("there is dirt within the range limit")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost/2, distanceForLost/2);
            state->SetCellTypeAt(dirtPos, CellType::DIRT);

            //do a generate for places all around the circle
            THEN("the heuristic doesn't kick in")
            {
                auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
                REQUIRE(cmd == nullptr);
            }

            AND_THEN("there is an enemy worm nearby")
            {
                auto enemyWormPos = playerWorm->position + Position(distanceForLost - 1, distanceForLost/2);
                place_worm(player1, 1, enemyWormPos, state);
                THEN("the heuristic doesn't kick in")
                {
                    auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
                    REQUIRE(cmd == nullptr);
                }   
            }
        }

        WHEN("there is no dirt at all")
        {
            THEN("the heuristic doesn't kick in")
            {
                auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
                REQUIRE(cmd == nullptr);
            }
        }

        WHEN("there is no enemy or dirt within the range limit (but dirt on the map)")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost+2, distanceForLost+5);
            state->SetCellTypeAt(dirtPos, CellType::DIRT);

            THEN("The heuristic kicks in")
            {
                auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
                REQUIRE(cmd != nullptr);

                AND_THEN("The given direction is correct")
                {
                    auto cmdStr = cmd->GetCommandString();
                    INFO(cmdStr)
                    REQUIRE( ((cmdStr == "move 0 2") || (cmdStr == "move 1 2") || (cmdStr == "move 2 1") || (cmdStr == "move 2 2")) ) ;
                }
            }

            AND_THEN("There is an ally close by")
            {
                auto friendlyPos = playerWorm->position + Position(distanceForLost - 1, distanceForLost/2);
                place_worm(player1, 2, friendlyPos, state);
                THEN("The heuristic kicks in")
                {
                    auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
                    REQUIRE(cmd != nullptr);

                    AND_THEN("The given direction is correct")
                    {
                        auto cmdStr = cmd->GetCommandString();
                        INFO(cmdStr)
                        REQUIRE( ((cmdStr == "move 0 2") || (cmdStr == "move 1 2") || (cmdStr == "move 2 1") || (cmdStr == "move 2 2")) ) ;
                    }
                }   
            }
        }
    }
}

TEST_CASE( "Heuristic to avoid getting lost - correct direction", "[GetNearestDirtHeuristicDirection]" )
{
    GIVEN("A contrived game state")
    {

        //    0   1   2   3   4 
        //0   .   .   .   .   .
        //1   .   11  .   .   .
        //2   .   .   .   .   .
        //3   .   .   .   .   .
        //4   .   .   .   .   .

        auto state = std::make_shared<GameState>();

        bool player1 = GENERATE(true, false);
        int distanceForLost = GENERATE(2, 5, 10, 25);

        Worm* playerWorm = place_worm(player1, 1, {1,1}, state);

        //bunch more for direction confirmation
        WHEN("We try to confirm direction...")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost+2, 0);
            state->SetCellTypeAt(dirtPos, CellType::DIRT);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);

            THEN("The given direction is correct")
            {
                auto cmdStr = cmd->GetCommandString();
                REQUIRE( ((cmdStr == "move 2 0") || (cmdStr == "move 2 1") || (cmdStr == "move 2 2")) ) ;
            }
        }
        WHEN("We try to confirm direction...")
        {
            auto dirtPos = playerWorm->position + Position(0, distanceForLost+2);
            state->SetCellTypeAt(dirtPos, CellType::DIRT);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
            
            THEN("The given direction is correct")
            {
                auto cmdStr = cmd->GetCommandString();
                REQUIRE( ((cmdStr == "move 0 2") || (cmdStr == "move 1 2") || (cmdStr == "move 2 2")) ) ;
            }
        }
        WHEN("We try to confirm direction...")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost + 1, distanceForLost+2);
            state->SetCellTypeAt(dirtPos, CellType::DIRT);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
            
            THEN("The given direction is correct")
            {
                auto cmdStr = cmd->GetCommandString();
                REQUIRE( ((cmdStr == "move 2 1") || (cmdStr == "move 2 2") || (cmdStr == "move 1 2")) ) ;
            }
        }
        WHEN("We try to confirm direction...")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost + 2, distanceForLost + 1);
            state->SetCellTypeAt(dirtPos, CellType::DIRT);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);
            
            THEN("The given direction is correct")
            {
                auto cmdStr = cmd->GetCommandString();
                REQUIRE( ((cmdStr == "move 2 1") || (cmdStr == "move 2 2") || (cmdStr == "move 1 2")) ) ;
            }
        }
    }
}


TEST_CASE( "Heuristic to avoid getting lost - avoid deep space", "[GetNearestDirtHeuristic_avoid_deepSpace]" )
{
    GIVEN("A game state")
    {
        auto state = std::make_shared<GameState>();

        bool player1 = GENERATE(true, false);
        int wormIndex = 1;
        int distanceForLost = 2;

        WHEN("We set things up for a crash (1)")
        {
            //    0   1   2   3   4 
            //0   D   .   .   .   .
            //1   .   .   .   .   .
            //2   .   .   .   .   .
            //3   .   .   .   .   .
            //4   S   .   .   .   .
            //5   S   11  .   .   .

            place_worm(player1, wormIndex, {1,5}, state);
            state->SetCellTypeAt({0,0}, CellType::DIRT);
            state->SetCellTypeAt({0,4}, CellType::DEEP_SPACE);
            state->SetCellTypeAt({0,5}, CellType::DEEP_SPACE);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);

            THEN("It doesn't crash us into dirt")
            {
                auto cmdStr = cmd->GetCommandString();
                REQUIRE( ( (cmdStr == "move 1 4") || (cmdStr == "move 2 4") ) ) ;
            }
        }

        WHEN("We set things up for a crash (2)")
        {
            //    0   1   2   3   4 
            //0   S   S   .   .   D
            //1   11  .   .   .   .
            //2   .   .   .   .   .
            //3   .   .   .   .   .
            //4   .   .   .   .   .
            //5   .   .   .   .   .

            place_worm(player1, wormIndex, {0,1}, state);
            state->SetCellTypeAt({4,0}, CellType::DIRT);
            state->SetCellTypeAt({1,0}, CellType::DEEP_SPACE);
            state->SetCellTypeAt({0,0}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);

            THEN("It doesn't crash us into dirt")
            {
                REQUIRE(cmd->GetCommandString() != "move 1 0");
                REQUIRE(cmd->GetCommandString() == "move 1 1");
            }
        }

        WHEN("We set things up for a crash (3)")
        {
            //    0   1   2   3   4 
            //0   .   .   .   .   .
            //1   .   .   .   .   .
            //2   .   .   .   .   .
            //3   .   .   .   .   .
            //4   11  .   .   .   .
            //5   S   S   .   .   D

            place_worm(player1, wormIndex, {0,4}, state);
            state->SetCellTypeAt({4,5}, CellType::DIRT);
            state->SetCellTypeAt({1,5}, CellType::DEEP_SPACE);
            state->SetCellTypeAt({0,5}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);

            THEN("It doesn't crash us into dirt")
            {
                auto cmdStr = cmd->GetCommandString();
                INFO(cmdStr)
                REQUIRE( ( (cmdStr == "move 1 4") || (cmdStr == "move 1 3") ) ) ;
            }
        }

        WHEN("We set things up for a crash (observed in match)")
        {
            //     0   1   2   3   4    5   6   7
            //21   12  .   .   .   .    .   .   .
            //22   S   .   .   .   .    .   .   .
            //23   S   .   .   .   .    .   .   .
            //24   S   .   .   .   .    .   .   .
            //25   S   S   .   .   .    .   .   .
            //26   S   S   S   .   .    .   .   .
            //27   S   S   S   .   .    .   .   .
            //28   S   S   S   .   .    .   .   .
            //29   S   S   S   .   .    .   .   .
            //30   S   S   S   .   .    .   .   D

            place_worm(player1, wormIndex, {0,21}, state);
            state->SetCellTypeAt({7,30}, CellType::DIRT);
            state->SetCellTypeAt({0,22}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);

            THEN("It doesn't crash us into dirt")
            {
                REQUIRE(cmd->GetCommandString() != "move 0 22");
                REQUIRE(cmd->GetCommandString() == "move 1 22");
            }
        }

        WHEN("We set things up for a crash into a worm (1)")
        {
            //    0   1   2   3   4 
            //0   D   .   .   .   .
            //1   .   .   .   .   .
            //2   .   .   .   .   .
            //3   .   .   .   .   .
            //4   12  .   .   .   .
            //5   S   11  .   .   .

            place_worm(player1, 1, {1,5}, state);
            place_worm(player1, 2, {0,4}, state);
            state->SetCellTypeAt({0,0}, CellType::DIRT);
            state->SetCellTypeAt({0,5}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);

            THEN("It doesn't crash us into the friendly worm")
            {
                REQUIRE(cmd->GetCommandString() != "move 0 4");
                REQUIRE(cmd->GetCommandString() == "move 1 4");
            }
        }

        WHEN("We set things up for a crash into a worm (2)")
        {
            //    0   1   2   3   4 
            //0   D   .   .   .   .
            //1   .   .   .   .   .
            //2   .   .   .   .   .
            //3   .   .   .   .   .
            //4   S   12  .   .   .
            //5   S   11  .   .   .

            place_worm(player1, 1, {1,5}, state);
            place_worm(player1, 2, {1,4}, state);
            state->SetCellTypeAt({0,0}, CellType::DIRT);
            state->SetCellTypeAt({0,4}, CellType::DEEP_SPACE);
            state->SetCellTypeAt({0,5}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);

            THEN("It doesn't crash us into the friendly worm")
            {
                auto cmdStr = cmd->GetCommandString();
                INFO(cmdStr)
                REQUIRE( ( (cmdStr == "move 2 4") ) ) ;
            }
        }

        WHEN("We set things up for a crash into a worm (3)")
        {
            //    0   1   2   3   4 
            //0   D   .   .   .   .
            //1   .   .   .   .   .
            //2   .   .   .   .   .
            //3   .   .   .   .   .
            //4   S   12  13  .   .
            //5   S   11  .   .   .
            //5   .   .   .   .   .

            place_worm(player1, 1, {1,5}, state);
            place_worm(player1, 2, {1,4}, state);
            place_worm(player1, 3, {2,4}, state);
            state->SetCellTypeAt({0,0}, CellType::DIRT);
            state->SetCellTypeAt({0,4}, CellType::DEEP_SPACE);
            state->SetCellTypeAt({0,5}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, state, distanceForLost);

            THEN("The heuristic does the best it can")
            {
                auto cmdStr = cmd->GetCommandString();
                INFO(cmdStr)
                REQUIRE( ( (cmdStr == "move 2 5") ) ) ;
            }
        }
    }
}





//TODO find out why it always does invalids at the end of the game





//just made this to confirm that random moves are actually random
TEST_CASE( "TryApplySelect", "[TryApplySelect]" )
{
    GIVEN("A semi realistic game state")
    {

        //    0   1   2   3   4   5   5   6   7   
        //0   .   .   .   .   .   .   .   .   .   
        //1   .   11  .   .   .   .   .   .   .   
        //2   .   .   .   .   .   .   .   .   .      
        //3   .   .   .   .   .   .   .   13  .   
        //4   .   .   .   .   .   .   .   .   .   
        //5   .   .   .   .   .   .   .   .   .   
        //6   .   .   21  .   .   .   .   .   .   
        //7   .   12  .   .   .   .   .   22  .   
        //8   .   .   .   .   .   .   .   .   .   
        //9   .   .   .   .   .   .   .   .   .   

        auto state = std::make_shared<GameState>();
        place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {1,7}, state);
        place_worm(true, 3, {6,3}, state);
        place_worm(false, 1, {2,6}, state);
        place_worm(false, 2, {6,7}, state);

        REQUIRE(state->player1.remainingWormSelections > 0);
        REQUIRE(state->player2.remainingWormSelections > 0);

        WHEN("our heuristic should kick in")
        {
            auto selectStatement = NextTurn::TryApplySelect(true, state);

            //worm 1's turn, we should select either worm 2 or 3
            THEN("It does")
            {
                INFO(selectStatement);
                REQUIRE( (selectStatement == "select 2;" || selectStatement == "select 3;") );

            }
            AND_THEN("It projects the game state forward so it looks like it's that guy's turn")
            {
                INFO("current worm: " << state->player1.GetCurrentWorm()->id);
                REQUIRE( (state->player1.GetCurrentWorm()->id == 2 || state->player1.GetCurrentWorm()->id == 3) );
            }
        }

        WHEN("We have no selects left")
        {
            state->player1.remainingWormSelections = 0;
            THEN("Heuristic should not kick in")
            {
                REQUIRE(NextTurn::TryApplySelect(true, state) == "");
            }
        }

        WHEN("our heuristic SHOULDN'T kick in")
        {
            GameEngine eng(state);
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("It doesn't")
            {
                //worm 2's turn - he is in trouble already
                REQUIRE(NextTurn::TryApplySelect(true, state) == "");
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
        state->SetCellTypeAt({0, 0}, CellType::DIRT);
        state->SetCellTypeAt({2, 0}, CellType::DIRT);
        state->SetCellTypeAt({1, 3}, CellType::DIRT);
        state->SetCellTypeAt({0, 1}, CellType::DEEP_SPACE);

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
