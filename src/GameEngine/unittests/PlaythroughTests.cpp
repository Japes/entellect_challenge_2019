#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "EvaluationFunctions.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Playthroughs - depth", "[playthrough][playthrough_depth]" )
{
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        
        place_worm(true, 1, {1,1}, state);
        place_worm(true, 2, {1,10}, state);
        place_worm(true, 3, {1,20}, state);
        place_worm(false, 1, {20,5}, state);
        place_worm(false, 2, {21,10}, state);
        place_worm(false, 3, {22,20}, state);

        auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, false);

        WHEN("We do a playthrough to a certain depth")
        {
            int roundBefore = state->roundNumber;
            int depth = 4;
            int plies = 0;
            eng.Playthrough(true, std::make_shared<DoNothingCommand>(), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);

            THEN("The game engine advances by that many rounds")
            {
                REQUIRE(state->roundNumber == roundBefore + depth);
            }
        }

        WHEN("We do a playthrough to a depth -1")
        {
            int depth = -1;
            REQUIRE(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS);
            int plies = 0;
            eng.Playthrough(true, std::make_shared<DoNothingCommand>(), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);

            THEN("The game engine advances until the end")
            {
                REQUIRE(eng.GetResult().result != GameEngine::ResultType::IN_PROGRESS);
            }
        }
    }
}

TEST_CASE( "Playthroughs - evaluation", "[playthrough][playthrough_evaluation]" )
{
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        
        place_worm(true, 1, {1,1}, state);
        place_worm(false, 1, {1,2}, state);

        //kill other worms
        state->player1.worms[1].health = -1;
        state->player1.worms[2].health = -1;
        state->player2.worms[1].health = -1;
        state->player2.worms[2].health = -1;

        int depth = 3;
        WHEN("We set it up so that player1 wins")
        {
            auto fakeNextMoveFn = [](bool player1, GameStatePtr state) -> std::shared_ptr<Command> { 
                if(player1) {
                    return std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S);
                } else {
                    return std::make_shared<DoNothingCommand>();
                }
            };

            REQUIRE(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS);
            int plies = 0;

            AND_THEN("We don't run the playthrough to the end of the game")
            {
                auto ret = eng.Playthrough(true, std::make_shared<DoNothingCommand>(), fakeNextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);

                THEN("We get something greater than 0.5")
                {
                    REQUIRE(ret > 0.5);
                    REQUIRE(ret < 1);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player2);
                }
            }

            AND_THEN("We run the playthrough till the end, player1 wins outright")
            {
                auto ret = eng.Playthrough(true, std::make_shared<DoNothingCommand>(), fakeNextMoveFn, EvaluationFunctions::ScoreComparison, -1, -1, plies);

                THEN("We get 1")
                {
                    REQUIRE(ret == 1);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player2);
                }
            }
        }


        WHEN("We set it up so that player2 wins")
        {
            auto fakeNextMoveFn = [](bool player1, GameStatePtr state)  -> std::shared_ptr<Command> { 
                if(!player1) {
                    return std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S);
                } else {
                    return std::make_shared<DoNothingCommand>();
                }
            };

            REQUIRE(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS);
            int plies = 0;

            AND_THEN("We don't run the playthrough to the end of the game")
            {
                auto ret = eng.Playthrough(true, std::make_shared<DoNothingCommand>(), fakeNextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);

                THEN("We get something less than 0.5")
                {
                    REQUIRE(ret < 0.5);
                    REQUIRE(ret > 0);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player1);
                }
            }

            AND_THEN("We run the playthrough till the end, player2 wins outright")
            {
                auto ret = eng.Playthrough(true, std::make_shared<DoNothingCommand>(), fakeNextMoveFn, EvaluationFunctions::ScoreComparison, -1, -1, plies);

                THEN("We get 0")
                {
                    REQUIRE(ret == 0);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player1);
                }
            }
        }
    }
}

TEST_CASE( "Playthroughs - radius of interest", "[playthrough][playthrough_radius]" )
{
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        
        int radiusToConsider = 3;

        place_worm(true, 1, {0,0}, state);
        place_worm(true, 2, {0,1}, state);
        place_worm(true, 3, {0,27}, state);
        place_worm(false, 1, {0,radiusToConsider + 1}, state); //just out
        place_worm(false, 2, {0,radiusToConsider}, state); //just in
        place_worm(false, 3, {0,28}, state);

        //worms always move east...
        auto nextMoveFn = [] (bool player1, GameStatePtr state) -> std::shared_ptr<Command> {
            Player* player = state->GetPlayer(player1);
            Worm* worm = player->GetCurrentWorm();
            return std::make_shared<TeleportCommand>(worm->position + Position(1,0) );
        };

        int depth = 9; //each worm moves 3 times
        int plies{0};
        WHEN("We run a playthrough only considering worms in a radius")
        {
            eng.Playthrough(true, nextMoveFn(true, state.get()), nextMoveFn, EvaluationFunctions::ScoreComparison, radiusToConsider, depth, plies);

            THEN("Only those worms move")
            {
                CHECK(state->player1.worms[0].position == Position(3,0));
                CHECK(state->player1.worms[1].position == Position(3,1));
                CHECK(state->player1.worms[2].position == Position(0,27));
                CHECK(state->player2.worms[0].position == Position(0,radiusToConsider + 1));
                CHECK(state->player2.worms[1].position == Position(3,radiusToConsider));
                CHECK(state->player2.worms[2].position == Position(0,28));
            }
        }

        WHEN("We run a playthrough considering all worms")
        {
            eng.Playthrough(true, nextMoveFn(true, state.get()), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);

            THEN("all worms move")
            {
                CHECK(state->player1.worms[0].position == Position(3,0));
                CHECK(state->player1.worms[1].position == Position(3,1));
                CHECK(state->player1.worms[2].position == Position(3,27));
                CHECK(state->player2.worms[0].position == Position(3,radiusToConsider + 1));
                CHECK(state->player2.worms[1].position == Position(3,radiusToConsider));
                CHECK(state->player2.worms[2].position == Position(3,28));
            }
        }
    }
}

TEST_CASE( "Playthroughs - radius of interest - maxDoNothings", "[playthrough][playthrough_radius_max_do_nothing]" )
{
    GIVEN("A semi realistic game state and engine with player2 almost disqualified")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        
        int radiusToConsider = 6;

        place_worm(true, 1, {0,0}, state);
        place_worm(true, 2, {0,1}, state);
        place_worm(true, 3, {0,2}, state);
        place_worm(false, 1, {0,28}, state);
        place_worm(false, 2, {0,29}, state);
        place_worm(false, 3, {0,30}, state);

        state->player2.consecutiveDoNothingCount = GameConfig::maxDoNothings - 1;

        auto nextMoveFn = [] (bool player1, GameStatePtr state) -> std::shared_ptr<Command> {
            Player* player = state->GetPlayer(player1);
            if(player->id == 1) {
                Worm* worm = player->GetCurrentWorm();
                return std::make_shared<TeleportCommand>(worm->position + Position(1,0) );
            } else {
                return std::make_shared<DoNothingCommand>();
            }
        };

        int depth = 30; //each worm moves 10 times
        int plies{0};
        WHEN("We run a playthrough only considering worms in a radius")
        {
            eng.Playthrough(true, nextMoveFn(true, state.get()), nextMoveFn, EvaluationFunctions::ScoreComparison, radiusToConsider, depth, plies);

            THEN("MaxDoNothings is ignored")
            {
                CHECK(state->player1.worms[0].position == Position(10,0));
                CHECK(state->player1.worms[1].position == Position(10,1));
                CHECK(state->player1.worms[2].position == Position(10,2));
                CHECK(state->player2.worms[0].position == Position(0,28));
                CHECK(state->player2.worms[1].position == Position(0,29));
                CHECK(state->player2.worms[2].position == Position(0,30));
                CHECK(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS);
            }
            
        }

        WHEN("We run a playthrough considering all worms")
        {
            eng.Playthrough(true, nextMoveFn(true, state.get()), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);
            
            THEN("MaxDoNothings is enforced")
            {
                CHECK(state->player1.worms[0].position.x < 10);
                CHECK(state->player1.worms[1].position.x < 10);
                CHECK(state->player1.worms[2].position.x < 10);
                CHECK(eng.GetResult().winningPlayer == &state->player1);
                CHECK(eng.GetResult().losingPlayer == &state->player2);
                CHECK(eng.GetResult().result == GameEngine::ResultType::FINISHED_POINTS);
            }
        }
    }
}
