#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "EvaluationFunctions.hpp"
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
            TeleportCommand player1move({10,8});
            DigCommand player2move({10,10});
            eng.AdvanceState(player1move, player2move);
            THEN("Its fine")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 0);
                REQUIRE(state->player2.consecutiveDoNothingCount == 0);
            }
        }

        WHEN("A player tries to move into something dug this round")
        {
            TeleportCommand player1move({10,10});
            DigCommand player2move({10,10});
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
            DigCommand player1move({10,10});
            ShootCommand player2move(ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);
            THEN("Its fine")
            {
                REQUIRE(targetWorm->health < GameConfig::commandoWorms.initialHp);
            }
        }

        WHEN("A player tries to shoot through dirt")
        {
            DigCommand player1move({9,9});
            ShootCommand player2move(ShootCommand::ShootDirection::N);
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
            TeleportCommand player1move({9,9});
            ShootCommand player2move(ShootCommand::ShootDirection::N);
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
            TeleportCommand player1move({9,9});
            DoNothingCommand player2doNothing;
            eng.AdvanceState(player1move, player2doNothing);
            //do another 2 turns so its their turn again
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            //now do the test

            player1move = TeleportCommand({10,9});
            ShootCommand player2shoot(ShootCommand::ShootDirection::N);
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
            TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
            TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
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
                    TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
                    TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
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
                            TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
                            TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
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
                                    TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
                                    TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
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
                                                TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
                                                TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
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
                                                        TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
                                                        TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
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
            DoNothingCommand player1move;
            TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 1; i < GameConfig::maxDoNothings; i++) {
                player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player1.consecutiveDoNothingCount == i);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player1 does nothing for 1 more turn") {
                player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
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
            TeleportCommand player1move({GameConfig::mapSize + 10, GameConfig::mapSize + 10} );
            TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 1; i < GameConfig::maxDoNothings; i++) {
                player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player1.consecutiveDoNothingCount == i);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player1 does nothing for 1 more turn") {
                player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
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
            DoNothingCommand player2move;
            TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 1; i < GameConfig::maxDoNothings; i++) {
                player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player2.consecutiveDoNothingCount == i);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player2 does nothing for 1 more turn") {
                player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
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
            TeleportCommand player2move({GameConfig::mapSize + 10, GameConfig::mapSize + 10} );
            TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 1; i < GameConfig::maxDoNothings; i++) {
                player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player2.consecutiveDoNothingCount == i);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player2 does nothing for 1 more turn") {
                player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
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

TEST_CASE( "Points are allocated correctly", "[scores]" ) {

    /*
    The total score value is determined by adding together the player's average worm health and the points for every single command the they played:

    Attack:
        Shooting any worm unconscious gives 40 points
        Shooting an enemy worm gives 20 points
        Shooting one of your own worms will reduce your points by 20
        A missed attack gives 2 points
    Moving gives 5 point
    Digging gives 7 points
    Doing nothing gives 0 points
    An invalid command will reduce your points by 4
    */

    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {0,0}, state);
        place_worm(true, 2, {2,2}, state);
        place_worm(true, 3, {0,6}, state);
        place_worm(false, 1, {0,8}, state);
        place_worm(false, 2, {0,12}, state);
        place_worm(false, 3, {0,15}, state);

        state->Cell_at({0, 1})->type = CellType::DIRT;
        state->Cell_at({0, 9})->type = CellType::DIRT;

        WHEN("both players do nothing")
        {
            DoNothingCommand player1move;
            DoNothingCommand player2move;
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.doNothing);
                CHECK(eng.GetResult().losingPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.doNothing);
            }
        }

        WHEN("both players just move")
        {
            TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{1,0});
            TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{1,0});
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.move);
                CHECK(eng.GetResult().losingPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.move);
            }
        }

        WHEN("both players just shoot")
        {
            ShootCommand player1move(ShootCommand::ShootDirection::E);
            ShootCommand player2move(ShootCommand::ShootDirection::E);
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.missedAttack);
                CHECK(eng.GetResult().losingPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.missedAttack);
            }
        }

        WHEN("both players just dig")
        {
            DigCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
            DigCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer != eng.GetResult().losingPlayer);
                CHECK(eng.GetResult().winningPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.dig);
                CHECK(eng.GetResult().losingPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.dig);
            }
        }

        WHEN("both players do an invalid command")
        {
            DigCommand player1move(state->player1.GetCurrentWorm()->position + Position{1,0});
            DigCommand player2move(state->player2.GetCurrentWorm()->position + Position{1,0});
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.invalidCommand);
                CHECK(eng.GetResult().losingPlayer->GetScore() == GameConfig::commandoWorms.initialHp + GameConfig::scores.invalidCommand);
            }
        }

        WHEN("player1 shoots a friendly, player2 shoots an enemy")
        {
            ShootCommand player1move(ShootCommand::ShootDirection::SE);
            ShootCommand player2move(ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                auto player1Score = state->player1.GetScore();
                auto player2Score = state->player2.GetScore();
                int expectedAverageWormHealth = (GameConfig::commandoWorms.initialHp*3 - GameConfig::commandoWorms.weapon.damage*2) / 3; //he gets hit by himself and player 2
                CHECK( player1Score == expectedAverageWormHealth + GameConfig::scores.friendlyFire);
                CHECK( player2Score == GameConfig::commandoWorms.initialHp + GameConfig::scores.attack);
            }
        }

        WHEN("player1 kills a friendly, plaer 2 kills an enemy")
        {
            state->player1.worms[1].health = 1;
            state->player1.worms[2].health = 1;
            ShootCommand player1move(ShootCommand::ShootDirection::SE);
            ShootCommand player2move(ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                auto player1Score = state->player1.GetScore();
                auto player2Score = state->player2.GetScore();
                int expectedAverageWormHealth = (GameConfig::commandoWorms.initialHp) / 3; //loses a guy to himself and to player 2
                CHECK( player1Score == expectedAverageWormHealth + GameConfig::scores.killShot);
                CHECK( player2Score == GameConfig::commandoWorms.initialHp + GameConfig::scores.killShot);
            }
        }

        //looks like scores aren't applied for powerups in their engine?
        //const int powerup =  20;
    }
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

        TeleportCommand player1move({0,0});
        TeleportCommand player2move({0,0});

        using resType = GameEngine::ResultType;

        REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);

        WHEN("game goes until maxturns - 1")
        {
            bool flipflop = false;
            for(unsigned i = 1; i < GameConfig::maxRounds; i++) {
                //make sure moves are valid
                if(flipflop) {
                    player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
                    player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
                } else {
                    player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{-1,-1});
                    player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{-1,-1});
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

        ShootCommand player1move(ShootCommand::ShootDirection::E);
        ShootCommand player2move(ShootCommand::ShootDirection::W);

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
            ShootCommand player1move(ShootCommand::ShootDirection::E);
            DoNothingCommand player2move;

            eng.AdvanceState(player1move, player2move);

            THEN("He wins") {
                REQUIRE(eng.GetResult().result == resType::FINISHED_KO);
                REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                REQUIRE(eng.GetResult().losingPlayer == &state->player2);
            }
        }

        WHEN("Player2 knocks out player1")
        {
            DoNothingCommand player1move;
            ShootCommand player2move(ShootCommand::ShootDirection::W);

            eng.AdvanceState(player1move, player2move);

            THEN("He wins") {
                REQUIRE(eng.GetResult().result == resType::FINISHED_KO);
                REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                REQUIRE(eng.GetResult().losingPlayer == &state->player1);
            }
        }
    }
}

TEST_CASE( "Playthroughs", "[playthrough]" )
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

        auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForWorm, std::placeholders::_1, std::placeholders::_2, false);

        WHEN("We do a playthrough to a certain depth")
        {
            int roundBefore = state->roundNumber;
            int depth = 4;
            eng.Playthrough(true, std::make_shared<DoNothingCommand>(), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth);

            THEN("The game engine advances by that many rounds")
            {
                REQUIRE(state->roundNumber == roundBefore + depth);
            }
        }

        WHEN("We do a playthrough to a depth -1")
        {
            int depth = -1;
            REQUIRE(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS);
            eng.Playthrough(true, std::make_shared<DoNothingCommand>(), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth);

            THEN("The game engine advances until the end")
            {
                REQUIRE(eng.GetResult().result != GameEngine::ResultType::IN_PROGRESS);
            }
        }


        int depth = 3;
        WHEN("We set it up so that player1 wins - (points diff improves)")
        {
            auto fakeNextMoveFn = [](bool player1, std::shared_ptr<GameState> state) -> std::shared_ptr<Command> { 
                if(player1) {
                    return std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S);
                } else {
                    return std::make_shared<DoNothingCommand>();
                }
            };

            REQUIRE(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS);
            int ret = eng.Playthrough(true, std::make_shared<DoNothingCommand>(), fakeNextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth);

            THEN("We get a positive result")
            {
                REQUIRE(ret == 1);
                REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                REQUIRE(eng.GetResult().losingPlayer == &state->player2);
            }
        }

        WHEN("We set it up so that player2 wins - (points diff improves)")
        {
            auto fakeNextMoveFn = [](bool player1, std::shared_ptr<GameState> state)  -> std::shared_ptr<Command> { 
                if(!player1) {
                    return std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S);
                } else {
                    return std::make_shared<DoNothingCommand>();
                }
            };

            REQUIRE(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS);
            int ret = eng.Playthrough(true, std::make_shared<DoNothingCommand>(), fakeNextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth);

            THEN("We get a negative result")
            {
                REQUIRE(ret == -1);
                REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                REQUIRE(eng.GetResult().losingPlayer == &state->player1);
            }
        }
    }
}
