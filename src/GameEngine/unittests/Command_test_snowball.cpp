#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Snowball validation", "[snowball]" ) {

    GIVEN("A game state and it's NOT technologist worm's turn")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {10,10}, state);
        place_worm(true, 2, {11,11}, state);
        place_worm(true, 3, {15,15}, state); //3 is technologist by default

        THEN("bomb's can't be lobbed")
        {
            SnowballCommand player1move({10,9});
            eng.AdvanceState(player1move, DoNothingCommand());
            REQUIRE(state->player1.consecutiveDoNothingCount == 1);
        }

        WHEN("It is technologist's turn")
        {
            //kill other worms
            state->player1.worms[0].health = -1;
            state->player1.worms[1].health = -1;
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            REQUIRE(state->player1.GetCurrentWorm()->proffession == Worm::Proffession::TECHNOLOGIST);

            THEN("He can lob a bomb")
            {
                SnowballCommand player1move({17,17});
                eng.AdvanceState(player1move, DoNothingCommand());
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            }

            WHEN("He has no bombs left") {
                //chuck 3
                SnowballCommand player1move({17,17});
                
                eng.AdvanceState(player1move, DoNothingCommand());
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                eng.AdvanceState(player1move, DoNothingCommand());
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                eng.AdvanceState(player1move, DoNothingCommand());
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);

                THEN("He can't lob a bomb")
                {
                    eng.AdvanceState(player1move, DoNothingCommand());
                    REQUIRE(state->player1.consecutiveDoNothingCount == 1);
                }
            }
        }
    }
}

TEST_CASE( "snowball range", "[snowball][snowball_range]" ) {

    //snowball radius:
    //0   1   2   3   4   5   6   7   8   9   10
    //0   .   .   .   .   .   .   .   .   .   .
    //1   .   x   x   x   x   .   .   .   .   .
    //2   .   x   x   x   x   x   .   .   .   .
    //3   .   x   x   x   x   x   x   .   .   .
    //4   .   x   x   x   x   x   x   .   .   .
    //5   .   x   x   x   x   x   x   .   .   .
    //6   .   W   x   x   x   x   x   .   .   .
    //7   .   .   .   .   .   .   .   .   .   .    

    GIVEN("A game state and it's the technologist worm's turn")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 2, {15,15}, state); //3 is always the technologist
        place_worm(false, 2, {29,29}, state); //put him somewhere sensible

        state->player1.worms[0].SetProffession(Worm::Proffession::TECHNOLOGIST);
        state->player1.worms[0].health = 1000; //so he doesn't kill himself during this test
        state->player1.worms[0].snowball_count = 1000; //its just a test


        auto wormpos = state->player1.GetCurrentWorm()->position;

        Position startPos = wormpos - Position(GameConfig::agentWorms.banana.range + 1, GameConfig::agentWorms.banana.range + 1);
        Position endPos = wormpos + Position(GameConfig::agentWorms.banana.range + 1, GameConfig::agentWorms.banana.range + 1);

        bool flipPlayer2Pos = 0;
        for(int x = startPos.x; x <= endPos.x; ++x) {
            for(int y = startPos.y; y <= endPos.y; ++y) {
                Position targetPos = {x,y};
                INFO("wormPos: " << wormpos << " targetPos: " << targetPos << " diff: " << targetPos - wormpos <<
                " shootDistance: " << wormpos.EuclideanDistanceTo(targetPos));

                SnowballCommand player1move(targetPos);

                //set up a valid move so that player2 doesn't get disqualified
                Worm* player2Worm = state->player2.GetCurrentWorm();
                Position player2moveTarget = flipPlayer2Pos? player2Worm->position + Position(-1,-1) : player2Worm->position + Position(1,1);
                TeleportCommand player2move(player2moveTarget);
                flipPlayer2Pos = !flipPlayer2Pos;

                state->player1.consecutiveDoNothingCount = 0;

                WHEN("We advance state")
                {
                    eng.AdvanceState(player1move, player2move);

                    THEN("The right places are throwable")
                    {
                        if(wormpos.EuclideanDistanceTo(targetPos) <= GameConfig::technologistWorms.snowball.range) {
                            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                        } else {
                            REQUIRE(state->player1.consecutiveDoNothingCount == 1);
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE( "Snowball can be lobbed over dirt", "[snowball]" ) {
    GIVEN("A setup with dirt")
    {
        //    0   1   2   3   4   5   6   7
        //0   D   D   D   D   D   D   D   .
        //1   D   D   D   D   D   D   D   .
        //2   D   D   D   D   D   D   D   .
        //3   D   D   D   D   D   D   D   .
        //4   D   D   D   D   D   D   D   .
        //5   .   .   .   .   .   W   D   .
        //6   .   .   .   .   .   .   D   .
        //7   .   .   .   .   .   .   D   .            

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);

        place_worm(true, 1, {5,5}, state);
        place_worm(true, 2, {26,26}, state);
        place_worm(true, 3, {25,25}, state);

        state->player1.worms[0].SetProffession(Worm::Proffession::TECHNOLOGIST);

        for(int x = 0; x < 7; ++x) {
            for(int y = 0; y < 5; ++y) {
                state->SetCellTypeAt({x, y}, CellType::DIRT);
            }
        }
        state->SetCellTypeAt({6, 5}, CellType::DIRT);
        state->SetCellTypeAt({6, 6}, CellType::DIRT);
        state->SetCellTypeAt({6, 7}, CellType::DIRT);

        WHEN("We chuck a snowball into empty space")
        {
            eng.AdvanceState(SnowballCommand({2,6}), DoNothingCommand());

            THEN("It's fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player1.worms[0].snowball_count == GameConfig::technologistWorms.snowball.count - 1);
            }
        }

        WHEN("We chuck a snowball into dirt")
        {
            eng.AdvanceState(SnowballCommand({2,2}), DoNothingCommand());

            THEN("It's fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player1.worms[0].snowball_count == GameConfig::technologistWorms.snowball.count - 1);
            }
        }

        WHEN("We chuck a snowball over dirt into empty space")
        {
            eng.AdvanceState(SnowballCommand({9,5}), DoNothingCommand());

            THEN("It's fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player1.worms[0].snowball_count == GameConfig::technologistWorms.snowball.count - 1);
            }
        }
    }
}

TEST_CASE( "Snowball bomb lobbed into deep space", "[snowball][deepspacebanana]" ) {
    //not invalid, but lose the bomb and does nothing
    GIVEN("A contrived situation")
    {
        //    0   1   2   3   4   5   6   7
        //0   S   S   S   S   S   S   .   .
        //1   S   S   S   S   S   S   .   .
        //2   S   S   S   S   S   S   .   .
        //3   S   S   S   S   S   S   .   .
        //4   S   S   S   S   S   S   .   .
        //5   .   .   W2  .   .   W   .   .
        //6   .   .   .   .   .   .   .   .
        //7   .   .   .   .   .   .   .   .            

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {5,5}, state);
        state->player1.worms[0].SetProffession(Worm::Proffession::TECHNOLOGIST);

        for(int x = 0; x < 7; ++x) {
            for(int y = 0; y < 5; ++y) {
                state->SetCellTypeAt({x, y}, CellType::DEEP_SPACE);
            }
        }
        place_worm(false, 1, {2,5}, state);

        REQUIRE(!state->player2.worms[0].IsFrozen());
        auto pointsBefore = state->player1.command_score;

        WHEN("We chuck a snowball into deep space")
        {
            eng.AdvanceState(SnowballCommand({2,4}), DoNothingCommand());

            THEN("It's fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player1.worms[0].snowball_count == GameConfig::technologistWorms.snowball.count - 1);
            }

            THEN("It doesn't actually go off")
            {
                REQUIRE(!state->player2.worms[0].IsFrozen());
                REQUIRE(state->player1.command_score == pointsBefore);
            }
        }
    }
}

TEST_CASE( "Snowball command: behavior", "[snowball]" ) {
    //check enemies get frozen
    //check friendlies get frozen
    //include some it should miss
    //confirm points - 17 per worm hit
    GIVEN("A contrived situation")
    {
        //    0   1   2   3   4   5   6   7
        //0   .   .   .   .   .   .   .   .
        //1   .   .   .   D   12  .   .   .
        //2   .   .   .   D   .   PU  .   .
        //3   .   .   .   D  B21  22  D   .
        //4   .   .   .   11 (23) .   D   .
        //5   .   .   .   .   .   D   D   .
        //6   .   13  .   .   D   D   .   .
        //7   .   .   .   .   .   .   .   .            

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {3,4}, state);
        place_worm(true, 2, {4,1}, state);
        place_worm(true, 3, {1,6}, state); //3 is technologist by default
        place_worm(false, 1, {4,3}, state);
        place_worm(false, 2, {5,3}, state);
        auto dedWorm = place_worm(false, 3, {4,4}, state);
        dedWorm->health = -1;

        Position powerupPos{5,2};
        place_powerup(powerupPos, state);

        state->SetCellTypeAt({3, 1}, CellType::DIRT);
        state->SetCellTypeAt({3, 2}, CellType::DIRT);
        state->SetCellTypeAt({3, 3}, CellType::DIRT);
        state->SetCellTypeAt({6, 3}, CellType::DIRT);
        state->SetCellTypeAt({6, 4}, CellType::DIRT);
        state->SetCellTypeAt({6, 5}, CellType::DIRT);
        state->SetCellTypeAt({5, 5}, CellType::DIRT);
        state->SetCellTypeAt({5, 6}, CellType::DIRT);
        state->SetCellTypeAt({4, 6}, CellType::DIRT);

        //make it technologist's turn
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

        Worm* currentWorm = state->player1.GetCurrentWorm();
        REQUIRE(currentWorm->proffession == Worm::Proffession::TECHNOLOGIST);

        auto pointsBefore = state->player1.command_score;
        REQUIRE(!state->player1.worms[0].IsFrozen());
        REQUIRE(!state->player1.worms[1].IsFrozen());
        REQUIRE(!state->player1.worms[2].IsFrozen());
        REQUIRE(!state->player2.worms[0].IsFrozen());
        REQUIRE(!state->player2.worms[1].IsFrozen());
        REQUIRE(!state->player2.worms[2].IsFrozen());

        REQUIRE(state->PowerUp_at(powerupPos) != nullptr);

        WHEN("We chuck the snowball")
        {
            eng.AdvanceState(SnowballCommand({4,3}), DoNothingCommand());

            THEN("Everything is as expected")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);

                //freeze:
                //P2
                CHECK(state->player2.worms[0].health == GameConfig::commandoWorms.initialHp);
                CHECK(state->player2.worms[0].IsFrozen());
                
                CHECK(state->player2.worms[1].health == GameConfig::agentWorms.initialHp);
                CHECK(state->player2.worms[1].IsFrozen());

                CHECK(state->player2.worms[2].IsDead());
                CHECK(!state->player2.worms[2].IsFrozen());

                //P1
                CHECK(state->player1.worms[0].health == GameConfig::commandoWorms.initialHp);
                CHECK(state->player1.worms[0].IsFrozen());
                
                CHECK(state->player1.worms[1].health == GameConfig::agentWorms.initialHp);
                CHECK(!state->player1.worms[1].IsFrozen());

                CHECK(state->player1.worms[2].health == GameConfig::technologistWorms.initialHp);
                CHECK(!state->player1.worms[2].IsFrozen());

                //dirt
                CHECK(state->CellType_at({3, 1}) == CellType::DIRT);
                CHECK(state->CellType_at({3, 2}) == CellType::DIRT);
                CHECK(state->CellType_at({3, 3}) == CellType::DIRT);
                CHECK(state->CellType_at({6, 3}) == CellType::DIRT);
                CHECK(state->CellType_at({6, 4}) == CellType::DIRT);
                CHECK(state->CellType_at({6, 5}) == CellType::DIRT);
                CHECK(state->CellType_at({5, 5}) == CellType::DIRT);
                CHECK(state->CellType_at({5, 6}) == CellType::DIRT);
                CHECK(state->CellType_at({4, 6}) == CellType::DIRT);

                //powerups not destroyed
                REQUIRE(state->PowerUp_at(powerupPos) != nullptr);

                //points
                auto expectedEnemyPoints = GameConfig::scores.freeze*2; //no points for ded guy
                auto expectedFriendlyPoints = -GameConfig::scores.freeze;
                CHECK(state->player1.command_score == pointsBefore + expectedEnemyPoints + expectedFriendlyPoints);
            }
        }
    }
}

TEST_CASE( "Freeze behaviour", "[snowball]" ) {
    //check that they cant do anything
    //the freeze counter goes down
    //once it's zero they can do shit again

    GIVEN("A contrived situation")
    {
        //    0   1   2   3   4   5   6
        //0  11   .   .   .   .   .   .
        //1  12   .   .   .   .   .   .
        //2   .   .   .   21  .   .   .
        //3   .   .   .  B22  .   .   .
        //4   .   .   .   23  .   .   .
        //5   .   .   .   .   .   .   .
        //6   13  .   .   .   .   .   .
        //7   .   .   .   .   .   .   .            

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);

        bool player1 = GENERATE(true, false);

        place_worm(player1, 1, {0,0}, state);
        place_worm(player1, 2, {0,1}, state);
        place_worm(player1, 3, {0,6}, state); //3 is technologist by default
        place_worm(!player1, 1, {3,2}, state);
        place_worm(!player1, 2, {3,3}, state);
        place_worm(!player1, 3, {3,4}, state);

        //make it technologist's turn
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

        Worm* currentWorm = state->player1.GetCurrentWorm();
        REQUIRE(currentWorm->proffession == Worm::Proffession::TECHNOLOGIST);

        REQUIRE(!state->player1.worms[0].IsFrozen());
        REQUIRE(!state->player1.worms[1].IsFrozen());
        REQUIRE(!state->player1.worms[2].IsFrozen());
        REQUIRE(!state->player2.worms[0].IsFrozen());
        REQUIRE(!state->player2.worms[1].IsFrozen());
        REQUIRE(!state->player2.worms[2].IsFrozen());

        Player* myPlayer = player1? &state->player1 : &state->player2;
        Player* enemyPlayer = player1? &state->player2 : &state->player1;

        WHEN("We chuck the snowball")
        {
            eng.AdvanceState(SnowballCommand({3,3}), DoNothingCommand());

            THEN("Enemies are all frozen")
            {
                CHECK(enemyPlayer->worms[0].IsFrozen());
                CHECK(enemyPlayer->worms[1].IsFrozen());
                CHECK(enemyPlayer->worms[2].IsFrozen());
            }

            AND_THEN("worms try to move for the next [freezeDuration] turns")
            {
                state->player1.consecutiveDoNothingCount = 0;
                state->player2.consecutiveDoNothingCount = 0;

                auto scoreBefore = enemyPlayer->command_score;

                for(int i = 1; i < GameConfig::technologistWorms.snowball.freezeDuration; ++i) { //this is how their engine seems to work...actually 4 turns
                    eng.AdvanceState(TeleportCommand(state->player1.GetCurrentWorm()->position + Position(1,0)), 
                                    TeleportCommand(state->player2.GetCurrentWorm()->position + Position(1,0)) );

                    REQUIRE(enemyPlayer->consecutiveDoNothingCount == 0); //snowballs aren't invalid
                }

                THEN("They can't")
                {
                    REQUIRE(enemyPlayer->worms[0].position.x == 3);
                    REQUIRE(enemyPlayer->worms[1].position.x == 3);
                    REQUIRE(enemyPlayer->worms[2].position.x == 3);
                }

                THEN("It doesn't count as an invalid move")
                {
                    REQUIRE(enemyPlayer->command_score == scoreBefore);
                }

                THEN("We can")
                {
                    REQUIRE(myPlayer->consecutiveDoNothingCount == 0);
                    REQUIRE(myPlayer->worms[0].position.x == 2);
                    REQUIRE(myPlayer->worms[1].position.x == 1);
                    REQUIRE(myPlayer->worms[2].position.x == 1);
                }

                AND_THEN("They try to move the turn after that")
                {
                    eng.AdvanceState(TeleportCommand(state->player1.GetCurrentWorm()->position + Position(1,0)), 
                                    TeleportCommand(state->player2.GetCurrentWorm()->position + Position(1,0)) );
                    THEN("They can")
                    {
                        REQUIRE(enemyPlayer->consecutiveDoNothingCount == 0);
                        REQUIRE(enemyPlayer->worms[1].position.x == 4);
                    }   
                }
            }
        }
    }
}

TEST_CASE( "Get snowball command string", "[snowball]" ) {
    SnowballCommand move(Position(5,6));
    REQUIRE(move.GetCommandString() == "snowball 5 6");

    SnowballCommand move2(Position(15,26));
    REQUIRE(move2.GetCommandString() == "snowball 15 26");
}
