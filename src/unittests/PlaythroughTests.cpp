#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "GameEngineTestUtils.hpp"
#include "Evaluators.hpp"

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
        auto evaluator = Evaluators::Score;

        WHEN("We do a playthrough to a certain depth")
        {
            int roundBefore = state->roundNumber;
            int depth = 4;
            int plies = 0;
            eng.Playthrough(std::make_shared<DoNothingCommand>(), std::make_shared<DoNothingCommand>(), nextMoveFn, evaluator, depth, plies);

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
            eng.Playthrough(std::make_shared<DoNothingCommand>(), std::make_shared<DoNothingCommand>(), nextMoveFn, evaluator, depth, plies);

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
        auto evaluator = Evaluators::Score;

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
                auto ret = eng.Playthrough(std::make_shared<DoNothingCommand>(), std::make_shared<DoNothingCommand>(), 
                                            fakeNextMoveFn, evaluator, depth, plies);

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
                auto ret = eng.Playthrough(std::make_shared<DoNothingCommand>(), std::make_shared<DoNothingCommand>(),
                                            fakeNextMoveFn, evaluator, -1, plies);

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
                auto ret = eng.Playthrough(std::make_shared<DoNothingCommand>(), std::make_shared<DoNothingCommand>(), 
                                            fakeNextMoveFn, evaluator, depth, plies);

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
                auto ret = eng.Playthrough(std::make_shared<DoNothingCommand>(), std::make_shared<DoNothingCommand>(), 
                                            fakeNextMoveFn, evaluator, -1, plies);

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
