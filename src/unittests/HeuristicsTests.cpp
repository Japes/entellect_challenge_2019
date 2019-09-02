#include "catch.hpp"
#include "GameConfig.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"
#include "GameEngineTestUtils.hpp"

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

        GameState state;
        GameEngine eng(&state);
        auto thrower = place_worm(true, 2, {9,6}, state);
        //make it the agent's turn...
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

        //big clump to the W
        state.SetCellTypeAt({6, 4}, CellType::DIRT);
        state.SetCellTypeAt({5, 5}, CellType::DIRT); state.SetCellTypeAt({6, 5}, CellType::DIRT); state.SetCellTypeAt({7, 5}, CellType::DIRT);
        state.SetCellTypeAt({4, 6}, CellType::DIRT); state.SetCellTypeAt({5, 6}, CellType::DIRT); state.SetCellTypeAt({6, 6}, CellType::DIRT); state.SetCellTypeAt({7, 6}, CellType::DIRT); state.SetCellTypeAt({8, 6}, CellType::DIRT);
        state.SetCellTypeAt({5, 7}, CellType::DIRT); state.SetCellTypeAt({6, 7}, CellType::DIRT); state.SetCellTypeAt({7, 7}, CellType::DIRT);
        state.SetCellTypeAt({6, 8}, CellType::DIRT);

        //sparse four to the NE
        state.SetCellTypeAt({10, 1}, CellType::DIRT); state.SetCellTypeAt({8, 3}, CellType::DIRT);
        state.SetCellTypeAt({12, 3}, CellType::DIRT); state.SetCellTypeAt({10, 5}, CellType::DIRT);

        //big clump to the S
        state.SetCellTypeAt({9, 9}, CellType::DIRT);
        state.SetCellTypeAt({8, 10}, CellType::DIRT); state.SetCellTypeAt({9, 10}, CellType::DIRT); state.SetCellTypeAt({10, 10}, CellType::DIRT);
        state.SetCellTypeAt({7, 11}, CellType::DIRT); state.SetCellTypeAt({8, 11}, CellType::DIRT); state.SetCellTypeAt({10, 11}, CellType::DIRT); state.SetCellTypeAt({11, 11}, CellType::DIRT);
        state.SetCellTypeAt({8, 12}, CellType::DIRT); state.SetCellTypeAt({9, 12}, CellType::DIRT); state.SetCellTypeAt({10, 12}, CellType::DIRT);
        state.SetCellTypeAt({9, 13}, CellType::DIRT);

        WHEN("He has bananas")
        {
            THEN("GetBananaMiningTargets returns correct")
            {
                auto ret = NextTurn::GetBananaMiningTargets(thrower, &state, 13);
                INFO("bananas: " << ret)
                REQUIRE(ret.count() == 1);
                REQUIRE(ret.test(57));
                REQUIRE(NextTurn::GetBanana(thrower, 57)->GetCommandString() == "banana 6 6");

            }

            THEN("GetBananaMiningTargets returns correct")
            {
                auto ret = NextTurn::GetBananaMiningTargets(thrower, &state, 12);
                INFO("bananas: " << ret)
                REQUIRE(ret.count() == 2);
                REQUIRE(ret.test(57));
                REQUIRE(NextTurn::GetBanana(thrower, 57)->GetCommandString() == "banana 6 6");
                REQUIRE(ret.test(115));
                REQUIRE(NextTurn::GetBanana(thrower, 115)->GetCommandString() == "banana 9 11");            
            }

            THEN("GetBananaMiningTargets returns correct")
            {
                auto ret = NextTurn::GetBananaMiningTargets(thrower, &state, 4);
                INFO("bananas: " << ret)
                REQUIRE(ret.test(28));
                REQUIRE(NextTurn::GetBanana(thrower, 28)->GetCommandString() == "banana 10 3");            
                REQUIRE(ret.count() > 6); //can see at LEAST 6 in that ascii picture
            }

            THEN("Banana prospecting returns correct")
            {
                auto bananaCommand = NextTurn::GetBananaProspect(true, &state, 12);
                REQUIRE(bananaCommand != nullptr);
                bool happy = bananaCommand->GetCommandString() == "banana 6 6" || bananaCommand->GetCommandString() == "banana 9 11";
                REQUIRE(happy);
            }

            THEN("Banana prospecting returns correct")
            {
                auto bananaCommand = NextTurn::GetBananaProspect(true, &state, 4);
                REQUIRE(bananaCommand != nullptr);
            }
        }

        WHEN("He has no bananas")
        {
            thrower->banana_bomb_count = 0;
            THEN("GetBananaMiningTargets returns zero")
            {
                auto ret = NextTurn::GetBananaMiningTargets(thrower, &state, 1);
                REQUIRE(ret.count() == 0);
            }

            THEN("Banana prospecting returns correct")
            {
                auto bananaCommand = NextTurn::GetBananaProspect(true, &state, 13);
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

        GameState state;

        bool player1 = GENERATE(true, false);
        int distanceForLost = GENERATE(2, 5, 10, 25);

        Worm* playerWorm = place_worm(player1, 1, {1,1}, state);

        WHEN("there is an enemy worm within the range limit")
        {
            auto enemyWormPos = playerWorm->position + Position(distanceForLost - 1, distanceForLost/2);
            place_worm(player1, 1, enemyWormPos, state);

            THEN("the heuristic doesn't kick in")
            {
                auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
                REQUIRE(cmd == nullptr);
            }
        }

        WHEN("there is dirt within the range limit")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost/2, distanceForLost/2);
            state.SetCellTypeAt(dirtPos, CellType::DIRT);

            //do a generate for places all around the circle
            THEN("the heuristic doesn't kick in")
            {
                auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
                REQUIRE(cmd == nullptr);
            }

            AND_THEN("there is an enemy worm nearby")
            {
                auto enemyWormPos = playerWorm->position + Position(distanceForLost - 1, distanceForLost/2);
                place_worm(player1, 1, enemyWormPos, state);
                THEN("the heuristic doesn't kick in")
                {
                    auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
                    REQUIRE(cmd == nullptr);
                }   
            }
        }

        WHEN("there is no dirt at all")
        {
            THEN("the heuristic doesn't kick in")
            {
                auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
                REQUIRE(cmd == nullptr);
            }
        }

        WHEN("there is no enemy or dirt within the range limit (but dirt on the map)")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost+2, distanceForLost+5);
            state.SetCellTypeAt(dirtPos, CellType::DIRT);

            THEN("The heuristic kicks in")
            {
                auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
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
                    auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
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

        GameState state;

        bool player1 = GENERATE(true, false);
        int distanceForLost = GENERATE(2, 5, 10, 25);

        Worm* playerWorm = place_worm(player1, 1, {1,1}, state);

        //bunch more for direction confirmation
        WHEN("We try to confirm direction...")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost+2, 0);
            state.SetCellTypeAt(dirtPos, CellType::DIRT);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

            THEN("The given direction is correct")
            {
                auto cmdStr = cmd->GetCommandString();
                REQUIRE( ((cmdStr == "move 2 0") || (cmdStr == "move 2 1") || (cmdStr == "move 2 2")) ) ;
            }
        }
        WHEN("We try to confirm direction...")
        {
            auto dirtPos = playerWorm->position + Position(0, distanceForLost+2);
            state.SetCellTypeAt(dirtPos, CellType::DIRT);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
            
            THEN("The given direction is correct")
            {
                auto cmdStr = cmd->GetCommandString();
                REQUIRE( ((cmdStr == "move 0 2") || (cmdStr == "move 1 2") || (cmdStr == "move 2 2")) ) ;
            }
        }
        WHEN("We try to confirm direction...")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost + 1, distanceForLost+2);
            state.SetCellTypeAt(dirtPos, CellType::DIRT);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
            
            THEN("The given direction is correct")
            {
                auto cmdStr = cmd->GetCommandString();
                REQUIRE( ((cmdStr == "move 2 1") || (cmdStr == "move 2 2") || (cmdStr == "move 1 2")) ) ;
            }
        }
        WHEN("We try to confirm direction...")
        {
            auto dirtPos = playerWorm->position + Position(distanceForLost + 2, distanceForLost + 1);
            state.SetCellTypeAt(dirtPos, CellType::DIRT);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);
            
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
        GameState state;

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
            state.SetCellTypeAt({0,0}, CellType::DIRT);
            state.SetCellTypeAt({0,4}, CellType::DEEP_SPACE);
            state.SetCellTypeAt({0,5}, CellType::DEEP_SPACE);
            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

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
            //1   11     .   .   .
            //2   .   .   .   .   .
            //3   .   .   .   .   .
            //4   .   .   .   .   .
            //5   .   .   .   .   .

            place_worm(player1, wormIndex, {0,1}, state);
            state.SetCellTypeAt({4,0}, CellType::DIRT);
            state.SetCellTypeAt({1,0}, CellType::DEEP_SPACE);
            state.SetCellTypeAt({0,0}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

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
            state.SetCellTypeAt({4,5}, CellType::DIRT);
            state.SetCellTypeAt({1,5}, CellType::DEEP_SPACE);
            state.SetCellTypeAt({0,5}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

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
            state.SetCellTypeAt({7,30}, CellType::DIRT);
            state.SetCellTypeAt({0,22}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

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
            state.SetCellTypeAt({0,0}, CellType::DIRT);
            state.SetCellTypeAt({0,5}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

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
            state.SetCellTypeAt({0,0}, CellType::DIRT);
            state.SetCellTypeAt({0,4}, CellType::DEEP_SPACE);
            state.SetCellTypeAt({0,5}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

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
            state.SetCellTypeAt({0,0}, CellType::DIRT);
            state.SetCellTypeAt({0,4}, CellType::DEEP_SPACE);
            state.SetCellTypeAt({0,5}, CellType::DEEP_SPACE);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

            THEN("The heuristic does the best it can")
            {
                auto cmdStr = cmd->GetCommandString();
                INFO(cmdStr)
                REQUIRE( ( (cmdStr == "move 2 5") ) ) ;
            }
        }

        WHEN("We put lava in the way")
        {
            //    0   1   2   3   4 
            //0   S   S   .   .   D
            //1   11  L   .   .   .
            //2   L   L   .   .   .
            //3   .   .   .   .   .
            //4   .   .   .   .   .
            //5   .   .   .   .   .

            place_worm(player1, wormIndex, {0,1}, state);
            state.SetCellTypeAt({4,0}, CellType::DIRT);
            state.SetCellTypeAt({1,0}, CellType::DEEP_SPACE);
            state.SetCellTypeAt({0,0}, CellType::DEEP_SPACE);
            state.AddLavaAt({1,1});
            state.AddLavaAt({1,2});
            state.AddLavaAt({0,2});

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

            THEN("It doesn't care")
            {
                REQUIRE(cmd->GetCommandString() != "move 1 0");
                REQUIRE(cmd->GetCommandString() == "move 1 1");
            }
        }

        WHEN("The dude is complete surrounded")
        {
            //    0   1   2   3   4 
            //0   S   S   .   .   D
            //1   11  D   .   .   .
            //2   W   D   .   .   .
            //3   .   .   .   .   .
            //4   .   .   .   .   .
            //5   .   .   .   .   .

            place_worm(player1, wormIndex, {0,1}, state);
            state.SetCellTypeAt({4,0}, CellType::DIRT);
            state.SetCellTypeAt({1,0}, CellType::DEEP_SPACE);
            state.SetCellTypeAt({0,0}, CellType::DEEP_SPACE);
            state.SetCellTypeAt({1,1}, CellType::DIRT);
            state.SetCellTypeAt({1,2}, CellType::DIRT);
            place_worm(!player1, wormIndex, {0,2}, state);

            auto cmd = NextTurn::GetNearestDirtHeuristic(player1, &state, distanceForLost);

            THEN("He gives up")
            {
                    REQUIRE(cmd == nullptr);
            }
        }
    }
}

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
        //6   .   .   .   .   .   .   .   .   .   
        //7   .   .   21  .   .   .   .   22  .   
        //8   .   12  .   .   .   .   .   .   .   
        //9   .   .   .   .   .   .   .   .   .   

        GameState state;
        place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {1,8}, state);
        place_worm(true, 3, {6,3}, state);
        place_worm(false, 1, {2,7}, state);
        place_worm(false, 2, {6,7}, state);

        REQUIRE(state.player1.remainingWormSelections > 0);
        REQUIRE(state.player2.remainingWormSelections > 0);

        WHEN("our heuristic should kick in")
        {
            auto selectStatement = NextTurn::TryApplySelect(true, &state, NextTurn::WormCanShoot);

            //worm 1's turn, we should select either worm 2 or 3
            THEN("It does")
            {
                INFO(selectStatement);
                REQUIRE( (selectStatement == "select 2;" || selectStatement == "select 3;") );

            }
            AND_THEN("It projects the game state forward so it looks like it's that guy's turn")
            {
                INFO("current worm: " << state.player1.GetCurrentWorm()->id);
                REQUIRE( (state.player1.GetCurrentWorm()->id == 2 || state.player1.GetCurrentWorm()->id == 3) );
            }
        }

        WHEN("We have no selects left")
        {
            state.player1.remainingWormSelections = 0;
            THEN("Heuristic should not kick in")
            {
                REQUIRE(NextTurn::TryApplySelect(true, &state, NextTurn::WormCanShoot) == "");
            }
        }

        WHEN("The dude is frozen")
        {
            state.player1.worms[1].roundsUntilUnfrozen = 5;
            state.player1.worms[2].roundsUntilUnfrozen = 5;
            THEN("Heuristic should not kick in")
            {
                REQUIRE(NextTurn::TryApplySelect(true, &state, NextTurn::WormCanShoot) == "");
            }
        }

        WHEN("our heuristic SHOULDN'T kick in")
        {
            GameEngine eng(&state);
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("It doesn't")
            {
                //worm 2's turn - he is in trouble already
                REQUIRE(NextTurn::TryApplySelect(true, &state, NextTurn::WormCanShoot) == "");
            }
        }
    }
}

TEST_CASE( "TryApplySelect - frozen dudes", "[TryApplySelectFrozenDudes]" )
{
    GIVEN("A semi realistic game state")
    {

        //    0   1   2   3   4   5   5   6   7   
        //0   .   .   .   .   .   .   .   .   .   
        //1   .   11  21  .   .   .   .   .   .   
        //2   .   .   .   .   .   .   .   .   .      
        //3   .   .   .   .   .   .   .   13  .   
        //4   .   .   .   .   .   .   .   .   .   
        //5   .   .   .   .   .   .   .   .   .   
        //6   .   .   .   .   .   .   .   .   .   
        //7   .   .   .  .   .   .   .   22  .   
        //8   .   12  .   .   .   .   .   .   .   
        //9   .   .   .   .   .   .   .   .   .   

        GameState state;
        place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {1,8}, state);
        place_worm(true, 3, {6,3}, state);
        place_worm(false, 1, {2,1}, state);
        place_worm(false, 2, {6,7}, state);


        REQUIRE(state.player1.remainingWormSelections > 0);
        REQUIRE(state.player2.remainingWormSelections > 0);

        WHEN("our heuristic should kick in")
        {
            state.player1.worms[0].roundsUntilUnfrozen = 5;
            auto selectStatement = NextTurn::TryApplySelect(true, &state, NextTurn::WormIsntFrozen);

            //worm 1's turn, we should select either worm 2 or 3
            THEN("It does")
            {
                INFO(selectStatement);
                REQUIRE( (selectStatement == "select 2;" || selectStatement == "select 3;") );
            }
        }

        WHEN("our heuristic shouldn't kick in")
        {
            state.player1.worms[0].roundsUntilUnfrozen = 0;

            THEN("It doesn't")
            {
                REQUIRE(NextTurn::TryApplySelect(true, &state, NextTurn::WormIsntFrozen) == "");
            }
        }
    }
}

TEST_CASE( "TryApplySelect bug", "[TryApplySelectBug]" )
{
    GIVEN("A game state that reproduces our bug")
    {

        //    0   1   2   3   4
        //0   .   .   .   .   .
        //1   .   13  .   .   .
        //2   .   .   .   .   .
        //3   .   .   .   .   .
        //4   .   .   .   .   .

        GameState state;
        bool player1 = GENERATE(true, false);
        auto DUTworm = place_worm(player1, 1, {1,1}, state);

        place_worm(player1, 2, {20,1}, state);
        place_worm(player1, 3, {20,2}, state);

        place_worm(!player1, 1, {30,5}, state);
        place_worm(!player1, 2, {30,3}, state);
        place_worm(!player1, 3, {30,4}, state);

        //make it so player1 will die if he does nothing
        DUTworm->health = 1;
        GameState::AddLavaAt({1,1});

        REQUIRE(state.player1.remainingWormSelections > 0);
        REQUIRE(state.player2.remainingWormSelections > 0);

        WHEN("we run the heuristic")
        {
            auto selectStatement = NextTurn::TryApplySelect(player1, &state, NextTurn::WormCanShoot);

            THEN("It doesn't lock things up...")
            {
                REQUIRE( true );
            }
        }
    }
}

