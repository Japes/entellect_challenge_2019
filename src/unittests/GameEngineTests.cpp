#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "I can make a game engine instance", "[sanity]" ) {
    GameEngine eng;
    REQUIRE( true );
}

TEST_CASE( "Commands are resolved in the right order", "[command_order]" ) {
    //according to the rules:
    //select
    //move
    //dig
    //banana
    //shoot

    GIVEN("A semi realistic game state and engine")
    {

       /*
            0   1   2   3   4   5   6   7   8   9   10
        0   .   .   .   .   .   .   .   .   .   .   .
        1   .   .   .   .   .   .   .   .   .   .   .
        2   12  .   .   .   .   .   .   .   .   .   .
        3   11  .   .   .   .   .   .   .   .   .   .
        4   .   .   .   .   .   .   .   .   .   .   .
        5   22  .   .   .   .   .   .   .   .   .   .
        6   21  .   .   .   .   .   .   .   .   .   .
        7   .   .   .   .   .   .   .   .   .   .   .
        8   .   .   .   .   .   .   .   .   .   .   .
        9   .   .   .   .   .   .   .   .   .   .   13
        10  .   .   .   .   .   .   .   .   B   .   D
        11  .   .   .   .   .   .   .   .   .   .   23
        */

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 2, {0,2}, state);
        place_worm(true, 1, {0,3}, state);
        place_worm(false, 2, {0,5}, state);
        place_worm(false, 1, {0,6}, state);

        place_worm(true, 3, {10,9}, state);
        place_worm(false, 3, {10,11}, state);
        state->SetCellTypeAt({10, 10}, CellType::DIRT);

        //make it the 3rd players turn
        state->player1.worms[2].SetProffession(Worm::Proffession::AGENT);
        state->player2.worms[2].SetProffession(Worm::Proffession::AGENT);
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
        state->player1.consecutiveDoNothingCount = 0;
        state->player2.consecutiveDoNothingCount = 0;

        REQUIRE(state->player1.GetCurrentWorm() == &state->player1.worms[2]);
        REQUIRE(state->player2.GetCurrentWorm() == &state->player2.worms[2]);
        REQUIRE(state->player1.consecutiveDoNothingCount == 0);
        REQUIRE(state->player2.consecutiveDoNothingCount == 0);

        //Move happens before dig----------------
        WHEN("A player moves and a player digs")
        {
            TeleportCommand player1move({10,8});
            DigCommand player2move({10,10});
            REQUIRE(state->player1.GetCurrentWorm() == &state->player1.worms[2]);
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
            REQUIRE(state->player1.GetCurrentWorm() == &state->player1.worms[2]);
            eng.AdvanceState(player1move, player2move);
            THEN("The move is evaluated before the dig")
            {
                REQUIRE(state->player1.consecutiveDoNothingCount == 1);
                REQUIRE(state->player2.consecutiveDoNothingCount == 0);
            }
        }

        //banana happens before shoot----------------
        WHEN("A player shoots and a player bananas")
        {
            state->player1.GetCurrentWorm()->health = 1;
            state->player2.GetCurrentWorm()->health = 1;

            BananaCommand player1move({8,11});
            ShootCommand player2move(ShootCommand::ShootDirection::N);

            eng.AdvanceState(player1move, player2move);

            THEN("The banana happens first")
            {
                REQUIRE(state->player2.worms[2].IsDead());
                REQUIRE(!state->player1.worms[2].IsDead());
            }
        }

        //Dig happens before shoot----------------
        Worm* targetWorm = state->player1.GetCurrentWorm();
        WHEN("A player digs and a player shoots")
        {
            REQUIRE(state->player1.GetCurrentWorm()->health == GameConfig::agentWorms.initialHp);
            DigCommand player1move({10,10});
            ShootCommand player2move(ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);
            THEN("Its fine")
            {
                REQUIRE(targetWorm->health < GameConfig::agentWorms.initialHp);
            }
        }

        WHEN("A player tries to shoot through dirt")
        {
            DigCommand player1move({9,9});
            ShootCommand player2move(ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);
            THEN("It fails")
            {
                REQUIRE(targetWorm->health == GameConfig::agentWorms.initialHp);
            }
        }

        //Move happens before shoot----------------
        WHEN("A player moves away from a shot")
        {
            REQUIRE(state->player1.GetCurrentWorm()->health == GameConfig::agentWorms.initialHp);

            state->SetCellTypeAt({10, 10}, CellType::AIR);
            TeleportCommand player1move({9,9});
            ShootCommand player2move(ShootCommand::ShootDirection::N);
            eng.AdvanceState(player1move, player2move);
            THEN("He doesn't get hit")
            {
                REQUIRE(targetWorm->health == GameConfig::agentWorms.initialHp);
            }
        }

        WHEN("A player moves into a shot")
        {
            state->SetCellTypeAt({10, 10}, CellType::AIR);
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
                REQUIRE(targetWorm->health < GameConfig::agentWorms.initialHp);
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

        WHEN("We advance state with a move down")
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

                WHEN("We advance state with a move down")
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

                        WHEN("We advance state with a move down")
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

                                WHEN("We advance state with a move down")
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

                                            WHEN("We advance state with a move down")
                                            {
                                                TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
                                                TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
                                                eng.AdvanceState(player1move, player2move);
                                                
                                                THEN("worm 2 for each player moves again")
                                                {
                                                    REQUIRE(state->player1.worms[0].position.y == 2);
                                                    REQUIRE(state->player1.worms[1].position.y == 2);
                                                    REQUIRE(state->player2.worms[0].position.y == 2);
                                                    REQUIRE(state->player2.worms[1].position.y == 2);

                                                    WHEN("We advance state with a move down")
                                                    {
                                                        TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{0,1});
                                                        TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{0,1});
                                                        eng.AdvanceState(player1move, player2move);
                                                        
                                                        THEN("worm 1 for each player moves again - not worm 3 coz hes dead!")
                                                        {
                                                            REQUIRE(state->player1.worms[0].position.y == 3);
                                                            REQUIRE(state->player1.worms[1].position.y == 2);
                                                            REQUIRE(state->player2.worms[0].position.y == 3);
                                                            REQUIRE(state->player2.worms[1].position.y == 2);
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

TEST_CASE( "13 do nothings means disqualified", "[disqualified]" ) {
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

        WHEN("Player1 does nothing for 12 turns")
        {
            DoNothingCommand player1move;
            TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 0; i < GameConfig::maxDoNothings; i++) {
                player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player1.consecutiveDoNothingCount == i+1);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player1 does nothing for 1 more turn") {
                player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and player2 wins") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_POINTS);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player1);
                }
            }
        }

        WHEN("Player1 does something invalid for 12 turns")
        {
            TeleportCommand player1move({GameConfig::mapSize + 10, GameConfig::mapSize + 10} );
            TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 0; i < GameConfig::maxDoNothings; i++) {
                player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player1.consecutiveDoNothingCount == i+1);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player1 does nothing for 1 more turn") {
                player2move = TeleportCommand(state->player2.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and player2 wins") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_POINTS);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player2);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player1);
                }
            }
        }

        WHEN("Player2 does nothing for 12 turns")
        {
            DoNothingCommand player2move;
            TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 0; i < GameConfig::maxDoNothings; i++) {
                player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player2.consecutiveDoNothingCount == i+1);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player2 does nothing for 1 more turn") {
                player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and player1 wins") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_POINTS);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player2);
                }
            }
        }

        WHEN("Player2 does something invalid for 12 turns")
        {
            TeleportCommand player2move({GameConfig::mapSize + 10, GameConfig::mapSize + 10} );
            TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{1,1});

            REQUIRE(state->player1.consecutiveDoNothingCount == 0);
            REQUIRE(state->player2.consecutiveDoNothingCount == 0);

            for(unsigned i = 0; i < GameConfig::maxDoNothings; i++) {
                player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                REQUIRE(state->player2.consecutiveDoNothingCount == i+1);
            }

            THEN("Game is still in progress") {
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Player2 does nothing for 1 more turn") {
                player1move = TeleportCommand(state->player1.GetCurrentWorm()->position + Position{1,1});
                eng.AdvanceState(player1move, player2move);
                THEN("Game is finished, and player1 wins") {
                    REQUIRE(eng.GetResult().result == resType::FINISHED_POINTS);
                    REQUIRE(eng.GetResult().winningPlayer == &state->player1);
                    REQUIRE(eng.GetResult().losingPlayer == &state->player2);
                }
            }
        }
    }
}

int ExpectedInitialHealth()
{
    return GameConfig::commandoWorms.initialHp + GameConfig::agentWorms.initialHp + GameConfig::technologistWorms.initialHp;
}

int ExpectedInitialHealthScore()
{
    return ExpectedInitialHealth()/3;
}

TEST_CASE( "Points are allocated correctly", "[scores]" ) {

    /*
    The total score value is determined by adding together the player's average worm health and the points for every single command the they played:

    Attack:
        Any damages dealt to enemies gives dmg*2 points
        Shooting a worm unconscious gives 40 points
        Shooting one of your own worms will reduce your points by 20
        A missed attack gives 2 points
    Moving gives 5 point
    Digging gives 7 points
    Doing nothing gives 0 points
    An invalid command will reduce your points by 4
    */

    GIVEN("A semi realistic game state and engine")
    {
        /*
            0   1   2   3   
        0   11  .   .   .   
        1   .   .   .   .   
        2   .   .   12  .   
        3   .   .   .   .   
        4   .   .   .   .   
        5   .   .   .   .   
        6   13  .   .   .   
        7   .   .   .   .   
        8   21  .   .   .   
        */

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {0,0}, state);
        place_worm(true, 2, {2,2}, state);
        place_worm(true, 3, {0,6}, state);
        place_worm(false, 1, {0,8}, state);
        place_worm(false, 2, {0,12}, state);
        place_worm(false, 3, {0,15}, state);

        state->SetCellTypeAt({0, 1}, CellType::DIRT);
        state->SetCellTypeAt({0, 9}, CellType::DIRT);

        WHEN("both players do nothing")
        {
            DoNothingCommand player1move;
            DoNothingCommand player2move;
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.doNothing);
                CHECK(eng.GetResult().losingPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.doNothing);
            }
        }

        WHEN("both players just move")
        {
            TeleportCommand player1move(state->player1.GetCurrentWorm()->position + Position{1,0});
            TeleportCommand player2move(state->player2.GetCurrentWorm()->position + Position{1,0});
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.move);
                CHECK(eng.GetResult().losingPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.move);
            }
        }

        WHEN("both players just shoot")
        {
            ShootCommand player1move(ShootCommand::ShootDirection::E);
            ShootCommand player2move(ShootCommand::ShootDirection::E);
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.missedAttack);
                CHECK(eng.GetResult().losingPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.missedAttack);
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
                CHECK(eng.GetResult().winningPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.dig);
                CHECK(eng.GetResult().losingPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.dig);
            }
        }

        WHEN("both players do an invalid command")
        {
            DigCommand player1move(state->player1.GetCurrentWorm()->position + Position{1,0});
            DigCommand player2move(state->player2.GetCurrentWorm()->position + Position{1,0});
            eng.AdvanceState(player1move, player2move);

            THEN("points are as expected")
            {
                CHECK(eng.GetResult().winningPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.invalidCommand);
                CHECK(eng.GetResult().losingPlayer->GetScore() == ExpectedInitialHealthScore() + GameConfig::scores.invalidCommand);
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
                int expectedAverageWormHealth = (ExpectedInitialHealth() - GameConfig::commandoWorms.weapon.damage*2) / 3; //he gets hit by himself and player 2
                CHECK( player1Score == expectedAverageWormHealth - GameConfig::commandoWorms.weapon.damage*2);
                CHECK( player2Score == ExpectedInitialHealthScore() + GameConfig::commandoWorms.weapon.damage*2);
            }
        }

        WHEN("player1 kills a friendly, player 2 kills an enemy")
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
                CHECK( player1Score == expectedAverageWormHealth - (GameConfig::scores.killShot + GameConfig::commandoWorms.weapon.damage*2)  );
                CHECK( player2Score == ExpectedInitialHealthScore() + (GameConfig::scores.killShot + GameConfig::commandoWorms.weapon.damage*2));
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
        place_worm(true, 1, {14,15}, state); //away from lava
        place_worm(true, 2, {2,11}, state);
        place_worm(true, 3, {2,21}, state);
        place_worm(false, 1, {16,15}, state); //away from lava
        place_worm(false, 2, {21,10}, state);
        place_worm(false, 3, {22,20}, state);

        DoNothingCommand nothing;

        using resType = GameEngine::ResultType;

        REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);

        WHEN("game goes until maxturns - 1")
        {
            for(unsigned i = 1; i < GameConfig::maxRounds; i++) {

                eng.AdvanceState(nothing, nothing);
                state->player1.consecutiveDoNothingCount = 0;
                state->player2.consecutiveDoNothingCount = 0;
            }

            THEN("Game is still in progress") {
                REQUIRE(state->roundNumber == GameConfig::maxRounds);
                REQUIRE(eng.GetResult().result == resType::IN_PROGRESS);
            }

            AND_THEN("Game goes for one more turn") {
                eng.AdvanceState(nothing, nothing);
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
            INFO("1player2 worm 0 health, pos: " << state->player2.worms[0].health << " " << state->player2.worms[0].position);
            eng.AdvanceState(player1move, player2move);

            REQUIRE(state->player1.worms[0].IsDead());
            REQUIRE(state->player1.worms[1].IsDead());
            REQUIRE(state->player1.worms[2].IsDead());

            INFO("player2 worm 0 health, pos: " << state->player2.worms[0].health << " " << state->player2.worms[0].position);
            REQUIRE(state->player2.worms[0].IsDead());
            REQUIRE(state->player2.worms[1].IsDead());
            REQUIRE(state->player2.worms[2].IsDead());

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

TEST_CASE( "If a worm is killed, everyone who damaged him that round gets kill points", "[killpoints]" ) {

    GIVEN("A contrived situation")
    {
        GameState state;
        GameEngine eng(&state);
        bool player1 = true;//GENERATE(true, false);

        Worm* dyingWorm = place_worm(player1, 2, {10,10}, state);
        place_worm(player1, 1, {13,10}, state);
        Worm* player2Worm = place_worm(!player1, 1, {5,10}, state);
        dyingWorm->health = 24; //just enough for a banana and a shot to kill him
        player2Worm->SetProffession(Worm::Proffession::AGENT);

        ShootCommand player1move(ShootCommand::ShootDirection::W);
        BananaCommand player2move({10,10});

        Player* player1Player = player1? &state.player1 : &state.player2;
        Player* player2Player = !player1? &state.player1 : &state.player2;
        int player1ScoreBefore = player1Player->command_score;
        int player2ScoreBefore = player2Player->command_score;

        WHEN("2 worms damage a guy and he dies that round")
        {
            if(player1) {
                eng.AdvanceState(player1move, player2move);
            } else {
                eng.AdvanceState(player2move, player1move);
            }

            THEN("Both of them get the killshot points") {
                CHECK(player1Player->command_score == player1ScoreBefore - (GameConfig::scores.killShot + GameConfig::commandoWorms.weapon.damage*2));
                CHECK(player2Player->command_score == player2ScoreBefore + (GameConfig::scores.killShot + GameConfig::agentWorms.banana.damage*2));
            }
        }
    }
}

TEST_CASE( "Reminder that we assume that banana and snowball have the same range", "[banana_snowball_reminder]" ) {
    REQUIRE(GameConfig::agentWorms.banana.range == GameConfig::technologistWorms.snowball.range);
    //if this fails, it is a reminder that you need to update BananaSnowballCanReach
}

TEST_CASE( "Reminder that we use a hardcoded diagRange value", "[diagRange_reminder]" ) {
    REQUIRE(GameConfig::commandoWorms.weapon.range == 4);
    REQUIRE(GameConfig::commandoWorms.weapon.diagRange == 3);
    //if this fails, it is a reminder that you need to update diagrange
}