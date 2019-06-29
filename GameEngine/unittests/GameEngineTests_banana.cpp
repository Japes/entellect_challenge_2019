#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Banana validation", "" ) {

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

TEST_CASE( "Banana range", "" ) {

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
        place_worm(true, 1, {10,10}, state);
        place_worm(true, 2, {11,11}, state);
        place_worm(true, 3, {15,15}, state); //3 is always the agent

        place_worm(false, 1, {10,1}, state);
        place_worm(false, 2, {11,1}, state);
        place_worm(false, 3, {15,1}, state); //3 is always the agent

        //make it agent's turn
        state->player1.worms[0].health = -1;
        state->player1.worms[1].health = -1;
        state->player1.worms[2].health = 1000; //so he doesn't kill himself during this test
        state->player1.worms[2].banana_bomb_count = 1000; //its just a test
        eng.AdvanceState(TeleportCommand({10,11}), DoNothingCommand());

        REQUIRE(state->player1.GetCurrentWorm()->proffession == Worm::Proffession::AGENT);
        REQUIRE(state->player1.consecutiveDoNothingCount == 0);

        auto wormpos = state->player1.GetCurrentWorm()->position;

        Position startPos = wormpos - Position(GameConfig::agentWorms.banana.range + 1, GameConfig::agentWorms.banana.range + 1);
        Position endPos = wormpos + Position(GameConfig::agentWorms.banana.range + 1, GameConfig::agentWorms.banana.range + 1);

        bool flipPlayer2Pos = 0;
        for(int x = startPos.x; x <= endPos.x; ++x) {
            for(int y = startPos.y; y <= endPos.y; ++y) {

                Position targetPos = {x,y};
                INFO("wormPos: " << wormpos << " targetPos: " << targetPos << " diff: " << targetPos - wormpos <<
                " shootDistance: " << wormpos.ShootDistanceTo(targetPos));

                BananaCommand player1move(targetPos);

                Worm* player2Worm = state->player2.GetCurrentWorm();
                Position player2moveTarget = flipPlayer2Pos? player2Worm->position + Position(-1,-1) : player2Worm->position + Position(1,1);
                TeleportCommand player2move(player2moveTarget);
                flipPlayer2Pos = !flipPlayer2Pos;

                state->player1.consecutiveDoNothingCount = 0;

                eng.AdvanceState(player1move, player2move);
                if(wormpos.ShootDistanceTo(targetPos) <= GameConfig::agentWorms.banana.range) {
                    REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                } else {
                    REQUIRE(state->player1.consecutiveDoNothingCount == 1);
                }
            }
        }
    }
}

TEST_CASE( "Banana can be lobbed over dirt", "" ) {
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
        */
    }
}

TEST_CASE( "Banana bomb lobbed into deep space", "" ) {
    //not invalid, but lose the bomb and does nothing
}

TEST_CASE( "Banana command: damage", "[Shoot_command]" ) {
    //to enemies
    //include some it should miss
    //confirm points
    //check different points in the damage radius
}

TEST_CASE( "Banana command: friendly fire", "[Shoot_command][Shot_missed]" ) {
    //to my dudes
    //confirm points
}

TEST_CASE( "Banana removes dirt correctly", "[Banana_command][banana_mining]" ) {

//check points here as well
}

TEST_CASE( "Banana command: mixed bag", "[Shoot_command][Shot_missed]" ) {

    //test a combo of friendly/enemy worms, and dirt
}

TEST_CASE( "Dead worms are removed correctly for banana", "[dead_worms]" ) {
    /*
    GIVEN("A target worm lined up for a kill")
    {
        auto state = std::make_shared<GameState>();

        Position shooting_worm_pos{10,10};
        Worm* shooting_worm = place_worm(true, 1, shooting_worm_pos, state);
        Position target_worm_pos{10,11};
        Worm* target_worm = place_worm(false, 1, target_worm_pos, state);
        target_worm->health = shooting_worm->weapon.damage;

        CHECK(!shooting_worm->IsDead());
        CHECK(!target_worm->IsDead());
        CHECK(state->Cell_at(shooting_worm_pos)->worm != nullptr);
        CHECK(state->Cell_at(target_worm_pos)->worm != nullptr);

        THEN("Shooting him kills him")
        {
            ShootCommand player1move(ShootCommand::ShootDirection::S);
            DoNothingCommand player2move;

            GameEngine eng(state);
            eng.AdvanceState(player1move,player2move);

            CHECK(!shooting_worm->IsDead());
            CHECK(target_worm->IsDead());
            CHECK(state->Cell_at(shooting_worm_pos)->worm != nullptr);
            CHECK(state->Cell_at(target_worm_pos)->worm == nullptr);
        }
    }*/
}

TEST_CASE( "Get command string", "[Banana_string]" ) {
    auto state = std::make_shared<GameState>();
    ShootCommand move(ShootCommand::ShootDirection::S);
    //REQUIRE(move.GetCommandString() == "shoot S");

    ShootCommand move1(ShootCommand::ShootDirection::NE);
    //REQUIRE(move1.GetCommandString() == "shoot NE");
}
