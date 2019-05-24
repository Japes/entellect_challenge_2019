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

void CheckWorm(Worm* worm, int id, int health, Position pos, Position last_pos, int moveRange, int digRange, int weaponDmg, int weaponRange)
{
    CHECK(worm->id == id);
    if(health > 0) {
        CHECK(worm->health == health);
    } else {
        CHECK(worm->health <= 0);
    }
    CHECK(worm->position == pos);
    CHECK(worm->previous_position == last_pos); //unknown from this file - should be set equal to currentPosition
    CHECK(worm->movementRange == moveRange);
    CHECK(worm->diggingRange == digRange);

    CHECK(worm->weapon.damage == weaponDmg);
    CHECK(worm->weapon.range == weaponRange);
    int readRange = worm->weapon.range;
    int expectedDiagRange = std::ceil(std::sqrt((readRange*readRange)/2));
    CHECK(worm->weapon.diagRange == expectedDiagRange);
}

void CheckCellEmpty(Cell* cell, CellType type)
{
    CHECK(cell->type == type );
    CHECK(cell->worm == nullptr );
    CHECK(cell->powerup == nullptr );
}

TEST_CASE( "Load GameState from rapidJSON object", "[IO]" ) {
    
    const std::string filePath = "./Test_files/state2.json";
    std::ifstream dataIn;
    dataIn.open(filePath, std::ifstream::in);
    REQUIRE(dataIn.is_open());

    std::stringstream buffer;
    buffer << dataIn.rdbuf();
    std::string stateJson = buffer.str();
    rapidjson::Document roundJSON;
    const bool parsed = !roundJSON.Parse(stateJson.c_str()).HasParseError();
    REQUIRE(parsed);

    GameState state(roundJSON);

    CHECK(state.roundNumber == 250);

    //should I load GameConfig stuff?  Maybe later...

    //this stuff might never change...
    int movementRange = 1;
    int diggingRange = 1;
    int weaponDmg = 8;
    int weaponRange = 4;

    CHECK(state.player1.id == 1);
    CHECK(state.player1.consecutiveDoNothingCount == 0); //unknown from this file - should be default starting
    CHECK(state.player1.currentWormId == 1); //unknown from this file - should be default starting
    CHECK(state.player1.health == 182); //implied from worm health
    CHECK(state.player1.score == 1847);

    CheckWorm(&state.player1.worms[0], 1, 0, Position(14, 27), Position(14, 27), movementRange, diggingRange, weaponDmg, weaponRange);
    CheckWorm(&state.player1.worms[1], 2, 54, Position(14, 27), Position(14, 27), movementRange, diggingRange, weaponDmg, weaponRange); //dunno htw pos is the same as worm 1 but ok
    CheckWorm(&state.player1.worms[2], 3, 128, Position(13, 27), Position(13, 27), movementRange, diggingRange, weaponDmg, weaponRange);

    CHECK(state.player2.id == 2);
    CHECK(state.player2.consecutiveDoNothingCount == 0);
    CHECK(state.player2.currentWormId == 2);
    CHECK(state.player2.health == 188);
    CHECK(state.player2.score == 1381);

    CheckWorm(&state.player2.worms[0], 1, -2, Position(30, 17), Position(30, 17), movementRange, diggingRange, weaponDmg, weaponRange);
    CheckWorm(&state.player2.worms[1], 2, 38, Position(12, 27), Position(12, 27), movementRange, diggingRange, weaponDmg, weaponRange);
    CheckWorm(&state.player2.worms[2], 3, 150, Position(7, 3), Position(7,3), movementRange, diggingRange, weaponDmg, weaponRange);

    //ill just do some spot checks on the map
    CheckCellEmpty(state.Cell_at({9, 0}), CellType::DEEP_SPACE);
    CheckCellEmpty(state.Cell_at({10, 0}), CellType::DEEP_SPACE);
    CheckCellEmpty(state.Cell_at({11, 0}), CellType::AIR);
    CheckCellEmpty(state.Cell_at({12, 0}), CellType::AIR);

    //player2 worm
    CheckCellEmpty(state.Cell_at({7, 2}), CellType::AIR);
    CHECK(state.Cell_at({7, 3})->type == CellType::AIR);
    //CHECK(state.Cell_at({7, 3})->powerup == nullptr);
    //CHECK(state.Cell_at({7, 3})->worm != nullptr);
    //CHECK(state.Cell_at({7, 3})->worm->id == 3);
    //CHECK(state.Cell_at({7, 3})->worm->health == 150);
    //CHECK(state.Cell_at({7, 3})->worm->position == Position(7,3));
    CheckCellEmpty(state.Cell_at({7, 4}), CellType::AIR);

    //player1 worm
    CHECK(state.Cell_at({13, 27})->type == CellType::AIR);
    CHECK(state.Cell_at({13, 27})->powerup == nullptr);
    CHECK(state.Cell_at({13, 27})->worm != nullptr);
    CHECK(state.Cell_at({13, 27})->worm->id == 3);
    CHECK(state.Cell_at({13, 27})->worm->health == 128);
    CHECK(state.Cell_at({13, 27})->worm->position == Position(13,27));

    //powerup and dirt
    CheckCellEmpty(state.Cell_at({10, 24}), CellType::AIR);
    CHECK(state.Cell_at({11, 24})->type == CellType::AIR);
    CHECK(state.Cell_at({11, 24})->worm == nullptr);
    CHECK(state.Cell_at({11, 24})->powerup != nullptr);
    CHECK(state.Cell_at({11, 24})->powerup->value == 10);
    CheckCellEmpty(state.Cell_at({12, 24}), CellType::DIRT);
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
    //test when worms have died
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

TEST_CASE( "Comparison with java engine", "[comparison]" ) {
    //read a bunch of known state files, moves, and expected outputs generated by their engine.
    //read in those files, apply the moves, and compare result with theirs.
}