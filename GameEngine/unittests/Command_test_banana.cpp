#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"

void SetupAgent(std::shared_ptr<GameState> state, GameEngine& eng)
{
    place_worm(true, 1, {10,10}, state);
    place_worm(true, 2, {11,11}, state);

    //make it agent's turn
    state->player1.worms[0].health = -1;
    state->player1.worms[1].health = -1;
    eng.AdvanceState(TeleportCommand({10,11}), DoNothingCommand());

    Worm* currentWorm = state->player1.GetCurrentWorm();
    REQUIRE(currentWorm->proffession == Worm::Proffession::AGENT);
    REQUIRE(currentWorm->banana_bomb_count == GameConfig::agentWorms.banana.count);
    REQUIRE(state->player1.consecutiveDoNothingCount == 0);
}

TEST_CASE( "Banana validation", "[banana]" ) {

    GIVEN("A game state and it's NOT agent worm's turn")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {10,10}, state);
        place_worm(true, 2, {11,11}, state);
        place_worm(true, 3, {15,15}, state); //3 is always the agent

        THEN("bomb's can't be lobbed")
        {
            BananaCommand player1move({10,9});
            eng.AdvanceState(player1move, DoNothingCommand());
            REQUIRE(state->player1.consecutiveDoNothingCount == 1);
        }

        WHEN("It is agent's turn")
        {
            //kill other worms
            state->player1.worms[0].health = -1;
            state->player1.worms[1].health = -1;
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            REQUIRE(state->player1.GetCurrentWorm()->proffession == Worm::Proffession::AGENT);

            THEN("He can lob a bomb")
            {
                BananaCommand player1move({17,17});
                eng.AdvanceState(player1move, DoNothingCommand());
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            }

            WHEN("He has no bombs left") {
                //chuck 3
                BananaCommand player1move({17,17});
                
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

TEST_CASE( "Banana range", "[banana]" ) {

    /*banana radius:
    0   1   2   3   4   5   6   7   8   9   10
    0   .   .   .   .   .   .   .   .   .   .
    1   .   x   x   x   x   .   .   .   .   .
    2   .   x   x   x   x   x   .   .   .   .
    3   .   x   x   x   x   x   x   .   .   .
    4   .   x   x   x   x   x   x   .   .   .
    5   .   x   x   x   x   x   x   .   .   .
    6   .   W   x   x   x   x   x   .   .   .
    7   .   .   .   .   .   .   .   .   .   .    
    */


    GIVEN("A game state and it's the agent worm's turn")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 3, {15,15}, state); //3 is always the agent

        SetupAgent(state,eng);
        state->player1.worms[2].health = 1000; //so he doesn't kill himself during this test
        state->player1.worms[2].banana_bomb_count = 1000; //its just a test

        auto wormpos = state->player1.GetCurrentWorm()->position;

        Position startPos = wormpos - Position(GameConfig::agentWorms.banana.range + 1, GameConfig::agentWorms.banana.range + 1);
        Position endPos = wormpos + Position(GameConfig::agentWorms.banana.range + 1, GameConfig::agentWorms.banana.range + 1);

        bool flipPlayer2Pos = 0;
        for(int x = startPos.x; x <= endPos.x; ++x) {
            for(int y = startPos.y; y <= endPos.y; ++y) {

                Position targetPos = {x,y};
                INFO("wormPos: " << wormpos << " targetPos: " << targetPos << " diff: " << targetPos - wormpos <<
                " shootDistance: " << wormpos.EuclideanDistanceTo(targetPos));

                BananaCommand player1move(targetPos);

                Worm* player2Worm = state->player2.GetCurrentWorm();
                Position player2moveTarget = flipPlayer2Pos? player2Worm->position + Position(-1,-1) : player2Worm->position + Position(1,1);
                TeleportCommand player2move(player2moveTarget);
                flipPlayer2Pos = !flipPlayer2Pos;

                state->player1.consecutiveDoNothingCount = 0;

                eng.AdvanceState(player1move, player2move);
                if(wormpos.EuclideanDistanceTo(targetPos) <= GameConfig::agentWorms.banana.range) {
                    REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                } else {
                    REQUIRE(state->player1.consecutiveDoNothingCount == 1);
                }
            }
        }
    }
}

TEST_CASE( "Banana can be lobbed over dirt", "[banana]" ) {
    GIVEN("A setup with dirt")
    {
        /*
            0   1   2   3   4   5   6   7
        0   D   D   D   D   D   D   D   .
        1   D   D   D   D   D   D   D   .
        2   D   D   D   D   D   D   D   .
        3   D   D   D   D   D   D   D   .
        4   D   D   D   D   D   D   D   .
        5   .   .   .   .   .   W   D   .
        6   .   .   .   .   .   .   D   .
        7   .   .   .   .   .   .   D   .            
        */

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {25,25}, state);
        place_worm(true, 2, {26,26}, state);
        place_worm(true, 3, {5,5}, state);
        SetupAgent(state, eng);
        for(int x = 0; x < 7; ++x) {
            for(int y = 0; y < 5; ++y) {
                state->SetCellTypeAt({x, y}, CellType::DIRT);
            }
        }
        state->SetCellTypeAt({6, 5}, CellType::DIRT);
        state->SetCellTypeAt({6, 6}, CellType::DIRT);
        state->SetCellTypeAt({6, 7}, CellType::DIRT);

        WHEN("We chuck a banana into empty space")
        {
            eng.AdvanceState(BananaCommand({2,6}), DoNothingCommand());

            THEN("It's fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player1.GetCurrentWorm()->banana_bomb_count == GameConfig::agentWorms.banana.count - 1);
            }
        }

        WHEN("We chuck a banana into dirt")
        {
            eng.AdvanceState(BananaCommand({2,2}), DoNothingCommand());

            THEN("It's fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player1.GetCurrentWorm()->banana_bomb_count == GameConfig::agentWorms.banana.count - 1);
            }
        }

        WHEN("We chuck a banana over dirt into empty space")
        {
            eng.AdvanceState(BananaCommand({9,5}), DoNothingCommand());

            THEN("It's fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player1.GetCurrentWorm()->banana_bomb_count == GameConfig::agentWorms.banana.count - 1);
            }
        }
    }
}

TEST_CASE( "Banana bomb lobbed into deep space", "[banana]" ) {
    //not invalid, but lose the bomb and does nothing
    GIVEN("A contrived situation")
    {
        /*
            0   1   2   3   4   5   6   7
        0   S   S   S   S   S   S   .   .
        1   S   S   S   S   S   S   .   .
        2   S   S   S   S   S   S   .   .
        3   S   S   S   S   S   S   .   .
        4   S   S   S   S   S   S   .   .
        5   .   D   W2  D   .   W   .   .
        6   .   .   .   .   .   .   .   .
        7   .   .   .   .   .   .   .   .            
        */

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 3, {5,5}, state);
        SetupAgent(state, eng);
        for(int x = 0; x < 7; ++x) {
            for(int y = 0; y < 5; ++y) {
                state->SetCellTypeAt({x, y}, CellType::DEEP_SPACE);
            }
        }
        state->SetCellTypeAt({1, 5}, CellType::DIRT);
        place_worm(false, 1, {2,5}, state);
        state->SetCellTypeAt({3, 5}, CellType::DIRT);

        WHEN("We chuck a banana into deep space")
        {
            eng.AdvanceState(BananaCommand({2,4}), DoNothingCommand());

            THEN("It's fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player1.GetCurrentWorm()->banana_bomb_count == GameConfig::agentWorms.banana.count - 1);
            }

            THEN("It doesn't actually go off")
            {
                REQUIRE(state->Cell_at({1, 5})->type == CellType::DIRT);
                REQUIRE(state->Cell_at({3, 5})->type == CellType::DIRT);
                REQUIRE(state->player2.worms[0].health == GameConfig::commandoWorms.initialHp);
            }
        }
    }
}

TEST_CASE( "Banana command: behavior", "[banana]" ) {
    //to enemies
    //include some it should miss
    //confirm points - do you get points just for lobbing a banana and missing completely?
        //going to assume no
    //check different points in the damage radius
    //Dead worms are removed correctly
    GIVEN("A contrived situation")
    {
        /*
            0   1   2   3   4   5   6   7
        0   .   .   .   .   .   .   .   .
        1   .   .   .   D   .   .   .   .
        2   .   .   .   D   12  .   .   .
        3   .   .   .   D  B21  22  D   .
        4   .   .   .   11  .   .   D   .
        5   .   .   .   .   23  D   D   .
        6   .   13  .   .   D   D   .   .
        7   .   .   .   .   .   .   .   .            
        */

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {3,4}, state);
        place_worm(true, 2, {4,2}, state);
        place_worm(true, 3, {1,6}, state); //3 is always the agent
        place_worm(false, 1, {4,3}, state);
        place_worm(false, 2, {5,3}, state);
        place_worm(false, 3, {4,5}, state);

        state->SetCellTypeAt({3, 1}, CellType::DIRT);
        state->SetCellTypeAt({3, 2}, CellType::DIRT);
        state->SetCellTypeAt({3, 3}, CellType::DIRT);
        state->SetCellTypeAt({6, 3}, CellType::DIRT);
        state->SetCellTypeAt({6, 4}, CellType::DIRT);
        state->SetCellTypeAt({6, 5}, CellType::DIRT);
        state->SetCellTypeAt({5, 5}, CellType::DIRT);
        state->SetCellTypeAt({5, 6}, CellType::DIRT);
        state->SetCellTypeAt({4, 6}, CellType::DIRT);

        //set up 2 kills
        state->player1.worms[1].health = 1;
        state->player2.worms[2].health = 1;


        //make it agent's turn
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

        Worm* currentWorm = state->player1.GetCurrentWorm();
        REQUIRE(currentWorm->proffession == Worm::Proffession::AGENT);

        auto pointsBefore = state->player1.command_score;

        WHEN("We chuck the banana")
        {
            eng.AdvanceState(BananaCommand({4,3}), DoNothingCommand());

            THEN("Everything is as expected")
            {
                //damage:
                //P2
                CHECK(state->player2.worms[0].health == GameConfig::commandoWorms.initialHp - GameConfig::agentWorms.banana.damage);
                CHECK(!state->player2.worms[0].IsDead());
                
                CHECK(state->player2.worms[1].health == GameConfig::commandoWorms.initialHp - 13);
                CHECK(!state->player2.worms[1].IsDead());

                CHECK(state->player2.worms[2].health == 1 - 7);
                CHECK(state->player2.worms[2].IsDead());
                CHECK(state->Cell_at(state->player2.worms[2].position)->worm == nullptr);

                //P1
                CHECK(state->player1.worms[0].health == GameConfig::commandoWorms.initialHp - 11);
                CHECK(!state->player1.worms[0].IsDead());

                CHECK(state->player1.worms[1].health == 1 - 13);
                CHECK(state->player1.worms[1].IsDead());
                CHECK(state->Cell_at(state->player1.worms[1].position)->worm == nullptr);
                
                CHECK(state->player1.worms[2].health == GameConfig::agentWorms.initialHp);
                CHECK(!state->player1.worms[2].IsDead());

                //dirt
                CHECK(state->Cell_at({3, 1})->type == CellType::DIRT);
                CHECK(state->Cell_at({3, 2})->type == CellType::AIR);
                CHECK(state->Cell_at({3, 3})->type == CellType::AIR);
                CHECK(state->Cell_at({6, 3})->type == CellType::AIR);
                CHECK(state->Cell_at({6, 4})->type == CellType::DIRT);
                CHECK(state->Cell_at({6, 5})->type == CellType::DIRT);
                CHECK(state->Cell_at({5, 5})->type == CellType::DIRT);
                CHECK(state->Cell_at({5, 6})->type == CellType::DIRT);
                CHECK(state->Cell_at({4, 6})->type == CellType::DIRT);

                //points
                auto expectedEnemyDmgPoints = (GameConfig::agentWorms.banana.damage + 13 + 7 + GameConfig::scores.killShot)*2;
                auto expectedFriendlyDmgPoints = (11 + 13 + GameConfig::scores.killShot)*-2;
                auto expectedDigPoints = GameConfig::scores.dig*3;
                CHECK(state->player1.command_score == pointsBefore + expectedEnemyDmgPoints + expectedFriendlyDmgPoints + expectedDigPoints);
            }
        }
    }
}

TEST_CASE( "Get command string", "[banana]" ) {
    auto state = std::make_shared<GameState>();
    ShootCommand move(ShootCommand::ShootDirection::S);
    //REQUIRE(move.GetCommandString() == "shoot S");

    ShootCommand move1(ShootCommand::ShootDirection::NE);
    //REQUIRE(move1.GetCommandString() == "shoot NE");
}
