#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "I can make a game engine instance", "[sanity]" ) {
    GameEngine eng;
    REQUIRE( true );
}

TEST_CASE( "Commands are resolved in the right order", "[command_order]" ) {
    //according to the rules:
    //move
    //dig
    //shoot

    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 2, {0,2}, state);
        place_worm(true, 3, {0,3}, state);
        place_worm(false, 2, {0,5}, state);
        place_worm(false, 3, {0,6}, state);

        place_worm(true, 1, {10,9}, state);
        place_worm(false, 1, {10,11}, state);
        state->Cell_at({10, 10})->type = CellType::DIRT;

        REQUIRE(state->player1.consecutiveDoNothingCount == 0);
        REQUIRE(state->player2.consecutiveDoNothingCount == 0);

        //Move happens before dig----------------
        WHEN("A player moves and a player digs")
        {
            TeleportCommand player1move(true, state, {10,8});
            DigCommand player2move(false, state, {10,10});
            eng.AdvanceState(player1move, player2move);
            THEN("Its fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player2.consecutiveDoNothingCount == 0);
            }
        }

        WHEN("A player tries to move into something dug this round")
        {
            TeleportCommand player1move(true, state, {10,10});
            DigCommand player2move(false, state, {10,10});
            eng.AdvanceState(player1move, player2move);
            THEN("The move is evaluated before the dig")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 1);
                REQUIRE(state->player2.consecutiveDoNothingCount == 0);
            }
        }

        //Dig happens before shoot----------------
        Worm* targetWorm = state->player1.GetCurrentWorm();

        REQUIRE(state->player1.GetCurrentWorm()->health == GameConfig::commandoWorms.initialHp);
        WHEN("A player digs and a player shoots")
        {
            DigCommand player1move(true, state, {10,10});
            ShootCommand player2move(false, state, ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);
            THEN("Its fine")
            {
                REQUIRE(targetWorm->health < GameConfig::commandoWorms.initialHp);
            }
        }

        WHEN("A player tries to shoot through dirt")
        {
            DigCommand player1move(true, state, {9,9});
            ShootCommand player2move(false, state, ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);
            THEN("It fails")
            {
                REQUIRE(targetWorm->health == GameConfig::commandoWorms.initialHp);
            }
        }

        //Move happens before shoot----------------
        REQUIRE(state->player1.GetCurrentWorm()->health == GameConfig::commandoWorms.initialHp);
        WHEN("A player moves away from a shot")
        {
            state->Cell_at({10, 10})->type = CellType::AIR;
            TeleportCommand player1move(true, state, {9,9});
            ShootCommand player2move(false, state, ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);
            THEN("He doesn't get hit")
            {
                REQUIRE(targetWorm->health == GameConfig::commandoWorms.initialHp);
            }
        }

        WHEN("A player moves into a shot")
        {
            state->Cell_at({10, 10})->type = CellType::AIR;
            //move player1 out of firing range
            TeleportCommand player1move(true, state, {9,9});
            DoNothingCommand player2doNothing(false, state);
            eng.AdvanceState(player1move, player2doNothing);
            //do another 2 turns so its their turn again
            eng.AdvanceState(DoNothingCommand(true, state), DoNothingCommand(false, state));
            eng.AdvanceState(DoNothingCommand(true, state), DoNothingCommand(false, state));

            //now do the test

            player1move = TeleportCommand(true, state, {10,9});
            ShootCommand player2shoot(false, state, ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2shoot);
            THEN("He gets hit")
            {
                REQUIRE(targetWorm->health < GameConfig::commandoWorms.initialHp);
            }
        }
    }
}

TEST_CASE( "Active worms are chosen correctly", "[active_worm]" ) {
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {0,0}, state);
        place_worm(true, 2, {1,0}, state);
        place_worm(true, 3, {2,0}, state);
        place_worm(false, 1, {3,0}, state);
        place_worm(false, 2, {4,0}, state);
        place_worm(false, 3, {5,0}, state);

        WHEN("We advancd state with a move to the right")
        {
            TeleportCommand player1move(true, state, state->player1.GetCurrentWorm()->position + Position{0,1});
            TeleportCommand player2move(false, state, state->player2.GetCurrentWorm()->position + Position{0,1});
            eng.AdvanceState(player1move, player2move);

            THEN("worm 1 for each player moves")
            {
                REQUIRE(state->player1.worms[0].position.y == 1);
                REQUIRE(state->player1.worms[1].position.y == 0);
                REQUIRE(state->player1.worms[2].position.y == 0);
                REQUIRE(state->player2.worms[0].position.y == 1);
                REQUIRE(state->player2.worms[1].position.y == 0);
                REQUIRE(state->player2.worms[2].position.y == 0);

                WHEN("We advancd state with a move to the right")
                {
                    TeleportCommand player1move(true, state, state->player1.GetCurrentWorm()->position + Position{0,1});
                    TeleportCommand player2move(false, state, state->player2.GetCurrentWorm()->position + Position{0,1});
                    eng.AdvanceState(player1move, player2move);
                    
                    THEN("worm 2 for each player moves")
                    {
                        REQUIRE(state->player1.worms[0].position.y == 1);
                        REQUIRE(state->player1.worms[1].position.y == 1);
                        REQUIRE(state->player1.worms[2].position.y == 0);
                        REQUIRE(state->player2.worms[0].position.y == 1);
                        REQUIRE(state->player2.worms[1].position.y == 1);
                        REQUIRE(state->player2.worms[2].position.y == 0);

                        WHEN("We advancd state with a move to the right")
                        {
                            TeleportCommand player1move(true, state, state->player1.GetCurrentWorm()->position + Position{0,1});
                            TeleportCommand player2move(false, state, state->player2.GetCurrentWorm()->position + Position{0,1});
                            eng.AdvanceState(player1move, player2move);
                            
                            THEN("worm 3 for each player moves")
                            {
                                REQUIRE(state->player1.worms[0].position.y == 1);
                                REQUIRE(state->player1.worms[1].position.y == 1);
                                REQUIRE(state->player1.worms[2].position.y == 1);
                                REQUIRE(state->player2.worms[0].position.y == 1);
                                REQUIRE(state->player2.worms[1].position.y == 1);
                                REQUIRE(state->player2.worms[2].position.y == 1);

                                WHEN("We advancd state with a move to the right")
                                {
                                    TeleportCommand player1move(true, state, state->player1.GetCurrentWorm()->position + Position{0,1});
                                    TeleportCommand player2move(false, state, state->player2.GetCurrentWorm()->position + Position{0,1});
                                    eng.AdvanceState(player1move, player2move);
                                    
                                    THEN("worm 1 for each player moves again")
                                    {
                                        REQUIRE(state->player1.worms[0].position.y == 2);
                                        REQUIRE(state->player1.worms[1].position.y == 1);
                                        REQUIRE(state->player1.worms[2].position.y == 1);
                                        REQUIRE(state->player2.worms[0].position.y == 2);
                                        REQUIRE(state->player2.worms[1].position.y == 1);
                                        REQUIRE(state->player2.worms[2].position.y == 1);

                                        THEN("we kill worm 3")
                                        {
                                            state->player1.worms[2].TakeDamage(5000);
                                            state->player2.worms[2].TakeDamage(5000);

                                            WHEN("We advancd state with a move to the right")
                                            {
                                                TeleportCommand player1move(true, state, state->player1.GetCurrentWorm()->position + Position{0,1});
                                                TeleportCommand player2move(false, state, state->player2.GetCurrentWorm()->position + Position{0,1});
                                                eng.AdvanceState(player1move, player2move);
                                                
                                                THEN("worm 2 for each player moves again")
                                                {
                                                    REQUIRE(state->player1.worms[0].position.y == 2);
                                                    REQUIRE(state->player1.worms[1].position.y == 2);
                                                    REQUIRE(state->player1.worms[2].position.y == 1);
                                                    REQUIRE(state->player2.worms[0].position.y == 2);
                                                    REQUIRE(state->player2.worms[1].position.y == 2);
                                                    REQUIRE(state->player2.worms[2].position.y == 1);

                                                    WHEN("We advancd state with a move to the right")
                                                    {
                                                        TeleportCommand player1move(true, state, state->player1.GetCurrentWorm()->position + Position{0,1});
                                                        TeleportCommand player2move(false, state, state->player2.GetCurrentWorm()->position + Position{0,1});
                                                        eng.AdvanceState(player1move, player2move);
                                                        
                                                        THEN("worm 1 for each player moves again - not worm 3 coz hes dead!")
                                                        {
                                                            REQUIRE(state->player1.worms[0].position.y == 3);
                                                            REQUIRE(state->player1.worms[1].position.y == 2);
                                                            REQUIRE(state->player1.worms[2].position.y == 1);
                                                            REQUIRE(state->player2.worms[0].position.y == 3);
                                                            REQUIRE(state->player2.worms[1].position.y == 2);
                                                            REQUIRE(state->player2.worms[2].position.y == 1);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE( "12 do nothings means disqualified", "[disqualified]" ) {
    using resType = GameEngine::ResultType;

    //check for player 1 and player 2
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {0,1}, state);
        place_worm(true, 2, {0,2}, state);
        place_worm(true, 3, {0,3}, state);
        place_worm(false, 1, {0,4}, state);
        place_worm(false, 2, {0,5}, state);
        place_worm(false, 3, {0,6}, state);

        //TODO make a function for this

        WHEN("Player1 does nothing for 11 turns")
        {
            DoNothingCommand player1move(true, state);
            TeleportCommand player2move(false, state, state->player2.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 1; i < GameConfig::maxDoNothings; i++) {
                player2move = TeleportCommand(false, state, state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player1.consecutiveDoNothingCount == i);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player1 does nothing for 1 more turn") {
                player2move = TeleportCommand(false, state, state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and player2 wins") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_KO);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player1);
                }
            }
        }

        WHEN("Player1 does something invalid for 11 turns")
        {
            TeleportCommand player1move(true, state, {GameConfig::mapSize + 10, GameConfig::mapSize + 10} );
            TeleportCommand player2move(false, state, state->player2.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 1; i < GameConfig::maxDoNothings; i++) {
                player2move = TeleportCommand(false, state, state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player1.consecutiveDoNothingCount == i);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player1 does nothing for 1 more turn") {
                player2move = TeleportCommand(false, state, state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and player2 wins") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_KO);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player1);
                }
            }
        }

        WHEN("Player2 does nothing for 11 turns")
        {
            DoNothingCommand player2move(false, state);
            TeleportCommand player1move(true, state, state->player1.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 1; i < GameConfig::maxDoNothings; i++) {
                player1move = TeleportCommand(true, state, state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player2.consecutiveDoNothingCount == i);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player2 does nothing for 1 more turn") {
                player1move = TeleportCommand(true, state, state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and player1 wins") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_KO);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player2);
                }
            }
        }

        WHEN("Player2 does something invalid for 11 turns")
        {
            TeleportCommand player2move(false, state, {GameConfig::mapSize + 10, GameConfig::mapSize + 10} );
            TeleportCommand player1move(true, state, state->player1.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 1; i < GameConfig::maxDoNothings; i++) {
                player1move = TeleportCommand(true, state, state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player2.consecutiveDoNothingCount == i);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player2 does nothing for 1 more turn") {
                player1move = TeleportCommand(true, state, state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and player1 wins") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_KO);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player2);
                }
            }
        }
    }
}

TEST_CASE( "Points are allocated correctly", "[command_order]" ) {
    //check for each thing mentioned in the rules and confirm game engine concurs
}

TEST_CASE( "Game ends when max rounds is reached", "[max_rounds]" ) {
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

        TeleportCommand player1move(true, state, {0,0});
        TeleportCommand player2move(false, state, {0,0});

        using resType = GameEngine::ResultType;

        REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);

        WHEN("game goes until maxturns - 1")
        {
            bool flipflop = false;
            for(unsigned i = 1; i < GameConfig::maxRounds; i++) {
                //make sure moves are valid
                if(flipflop) {
                    player1move = TeleportCommand(true, state, state->player1.GetCurrentWorm()->position + Position{1,1});
                    player2move = TeleportCommand(false, state, state->player2.GetCurrentWorm()->position + Position{1,1});
                } else {
                    player1move = TeleportCommand(true, state, state->player1.GetCurrentWorm()->position + Position{-1,-1});
                    player2move = TeleportCommand(false, state, state->player2.GetCurrentWorm()->position + Position{-1,-1});
                }
                eng.AdvanceState(player1move, player2move);
                flipflop = !flipflop;
            }

            THEN("Game is still in progress") {
                REQUIRE(state->roundNumber == GameConfig::maxRounds);
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Game goes for one more turn") {
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and went to score") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_POINTS);
                }
            }
        }
    }
}

TEST_CASE( "Game goes to score if both players die in the same round", "[double_KO]" ) {
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        state->player1.worms[1].health = 0;
        state->player1.worms[2].health = 0;
        state->player2.worms[1].health = 0;
        state->player2.worms[2].health = 0;
        Worm* worm1 = place_worm(true, 1, {9,10}, state);
        Worm* worm2 = place_worm(false, 1, {10,10}, state);
        worm1->health = 1;
        worm2->health = 1;

        ShootCommand player1move(true, state, ShootCommand::ShootDirection::E);
        ShootCommand player2move(false, state, ShootCommand::ShootDirection::W);

        using resType = GameEngine::ResultType;

        WHEN("players knock each other out in the same round")
        {
            eng.AdvanceState(player1move, player2move);

            THEN("Game is finished, and goes to score") {
                REQUIRE(eng.GetResult().result == resType::FINISHED_POINTS);
            }
        }
    }
}

TEST_CASE( "Correct player wins on a knockout", "[KO]" ) {
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        state->player1.worms[1].health = 0;
        state->player1.worms[2].health = 0;
        state->player2.worms[1].health = 0;
        state->player2.worms[2].health = 0;
        Worm* worm1 = place_worm(true, 1, {9,10}, state);
        Worm* worm2 = place_worm(false, 1, {10,10}, state);
        worm1->health = 1;
        worm2->health = 1;

        using resType = GameEngine::ResultType;

        WHEN("Player1 knocks out player2")
        {
            ShootCommand player1move(true, state, ShootCommand::ShootDirection::E);
            DoNothingCommand player2move(false, state);

            eng.AdvanceState(player1move, player2move);

            THEN("He wins") {
                REQUIRE(eng.GetResult().result == resType::FINISHED_KO);
                REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                REQUIRE(eng.GetResult().losingPlayer == &state->player2);
            }
        }

        WHEN("Player2 knocks out player1")
        {
            DoNothingCommand player1move(true, state);
            ShootCommand player2move(false, state, ShootCommand::ShootDirection::W);

            eng.AdvanceState(player1move, player2move);

            THEN("He wins") {
                REQUIRE(eng.GetResult().result == resType::FINISHED_KO);
                REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                REQUIRE(eng.GetResult().losingPlayer == &state->player1);
            }
        }
    }
}
